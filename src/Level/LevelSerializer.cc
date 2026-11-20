#include "LevelSerializer.hh"

// (it might look like cursor/byte_count are simplifiable, but tiles are written
//  bit-aligned instead of byte-aligned, so I think this is nicer)

const uint8_t BITS_IN_DELTA=2, MAX_DELTA=0b11;
const uint8_t BITS_IN_KIND =5, MAX_KIND =0b11111 ;
const uint64_t MAX_TILES_IN_LEVEL     = 64*32; // must be high upper bound

// ======= The impl =======

typedef enum {
  HEADERS_END = 0xF5, TILES_END = 0xF4,
  HEIGHT = 0xF6,
  WIDTH = 0xF7,
  NAME = 0xF8,
  GOAL = 0xF9,
  BACKGROUND = 0xFA,
  WATER_LEVEL = 0xFB,
  MODIFDATE = 0xFC,
  VARIANTS_START = 0xFD,
  VARIANTS_END = 0xFE
} HDF_SERDE_TAG;

typedef struct {
  uint8_t *output;      // output buffer
  size_t cursor;        // cursor into output buffer
  uint64_t* bit_count;  // where the bit count is stored (is NULL iff o is written to)
} HDF_SER_STATE;

void w(HDF_SER_STATE* s, uint8_t data) {
  if (s->bit_count) *s->bit_count += 8;
  else     s->output[s->cursor++]  = data;
}

void w_header(HDF_SER_STATE* s, uint8_t header_tag, uint8_t data) {
  w(s,header_tag); w(s,data);
}


void w_tile(HDF_SER_STATE* s, uint8_t value, uint8_t bitwidth) {
  if (s->bit_count) {
    *s->bit_count += bitwidth;
    return;
  }

  for (size_t i = 0; i < bitwidth; ++i) {
    uint8_t boolean = ((value >> i) & 0b1) == 1;

    size_t aligned_cur        = s->cursor / 8;
    size_t offset_inside_byte = s->cursor % 8;

    s->output[aligned_cur] ^= (boolean << offset_inside_byte); // do I assume the input is zero'd out? hmm
    //uint8_t byte = s->output[aligned_cur];
    //if (boolean) s->output[aligned_cur] |= (uint8_t)  (1 << offset_inside_byte);
    //else         s->output[aligned_cur] &= (uint8_t) ~(1 << offset_inside_byte);

    s->cursor += 1;
  }
}

// A run is ended by a tile whose index would be zero.
// (I wanted to use a specific tag, but not being byte-aligned makes this very hard)
uint64_t w_tile_run(HDF_SER_STATE* s, size_t i, size_t delta, uint8_t* input_tile_kinds) {
  // delta := sum ([however many MAX_DELTAs]) + first one that isn't MAX_DELTA

  while (delta >= MAX_DELTA){
      w_tile(s, MAX_DELTA, BITS_IN_DELTA);
      delta -= MAX_DELTA;
  }
  w_tile(s, delta, BITS_IN_DELTA); // if we had a multiple of MAX_DELTA, we add the additive identity to finish the run
                                   // else, we never wrote anything, so we write everything. either way, we gotta write out the remainder
  while (input_tile_kinds[i] != 0) {
    w_tile(s, input_tile_kinds[i], BITS_IN_KIND);
    i += 1;
  }
  w_tile(s, 0, BITS_IN_KIND); // termination marker (ig also the zero we just saw, lmao)

  return i+1;
}

HDF_SERDE_RESULT serialize(SerdeLevel level, uint8_t* output, uint64_t* bit_count) {
  HDF_SER_STATE state = { .output = output, .cursor = 0, .bit_count = bit_count };
  HDF_SER_STATE* s = &state;

  // --- Prelude ---
  w(s, 'D');
  w(s, 'O');
  w(s, 'G');

  w(s, level.version_major);
  w(s, level.version_minor);

  // --- Headers ---
  if (level.name_length > MAX_BYTES_IN_LEVELNAME) return LEVEL_NAME_TOO_LONG;

  w(s, NAME); w(s, level.name_length); for (size_t i=0;i<level.name_length;++i) w(s, level.name[i]);
  w_header(s, HEIGHT,       level.height);
  w_header(s, WIDTH,        level.width);
  w_header(s, GOAL,         level.goal);
  w_header(s, BACKGROUND,   level.background);
  w_header(s, WATER_LEVEL,  level.water_level);
  w(s, MODIFDATE); for (size_t i=0;i<sizeof(level.modification_date);++i) w(s, (level.modification_date>>(i*8))&0xFF);

  w(s, HEADERS_END);

  // --- Tiles ---
  s->cursor *= 8; // in bits now !!!

  size_t num_tiles = level.height * level.width;

  uint64_t i = 0;
  uint64_t delta = 0;
  while (i < num_tiles) {
    if (level.tiles_kind[i] == 0) {
        ++i;
        ++delta;
    } else {
        i = w_tile_run(s, i, delta, level.tiles_kind);
        delta = 1;
    }
  }
  w_tile(s, 0, BITS_IN_DELTA); // delta of zero => we're done

  s->cursor = (s->cursor+4)/8; // in bytes again !!

  // --- Variants ---
  w(s,VARIANTS_START);
  // TODO: TODOTODO
  w(s,VARIANTS_END);

  if (s->bit_count)  *s->bit_count = (*s->bit_count+4)/8;

  return OK;
}

typedef struct {
  uint8_t* input;    // input bytestream
  size_t cursor;      // cursor into input buffer
  SerdeLevel* level;       // output
} HDF_DE_STATE;

// reads entire byte from input
uint8_t r(HDF_DE_STATE* s) {
  uint8_t data = s->input[s->cursor];
  s->cursor++; // i'm scared of "you changed it twice in one line! UB upon ye!"
  return data;
}

// reads next tile info from input
uint8_t r_tileinfo(HDF_DE_STATE* s, uint8_t bitwidth) {
  uint8_t ret = 0;
  for (size_t i = 0; i < bitwidth; ++i) {
    size_t aligned_cur    = s->cursor / 8;
    size_t offset_in_byte = s->cursor % 8;

    uint8_t byte = s->input[aligned_cur];
    uint8_t boolean = (byte >> offset_in_byte) & 0b1;

    ret |= boolean << i;

    s->cursor += 1;
  }
  return ret;
}


void r_storetile(HDF_DE_STATE*s, size_t* tile_index, uint8_t kind) {
  s->level->tiles_kind[*tile_index] = kind;
  *tile_index += 1;
}

HDF_SERDE_RESULT deserialize(uint8_t* input, size_t input_length, SerdeLevel* level_output) {
  if (level_output == NULL) return OUTPUT_LEVEL_PTR_WAS_NULL;
  HDF_DE_STATE state = { .input = input, .cursor = 0, .level = level_output };
  HDF_DE_STATE* s = &state;

  if (input_length < 3) return NO_MAGIC_BYTES_FOUND;
  if (!(input[0] == 'D' && input[1] == 'O' && input[2] == 'G')) return NO_MAGIC_BYTES_FOUND;
  s->cursor = 3;

  s->level->version_major = r(s);
  s->level->version_minor = r(s);

  // --- Headers ---
  uint8_t currently_reading_headers = 1;
  while (currently_reading_headers) {
    switch (r(s)) {
      case NAME: { // We trust the caller followed the precondition
        s->level->name_length = r(s); if (s->level->name_length > MAX_BYTES_IN_LEVELNAME) return LEVEL_NAME_TOO_LONG;
        for (size_t i=0;i<s->level->name_length;++i) s->level->name[i]=r(s);
        break;
      };
      case MODIFDATE: {
        s->level->modification_date = 0;
        for (size_t i=0;i<4;++i) s->level->modification_date |= (r(s) << (i*8));
        break;
      }
      case HEIGHT:      s->level->height      = r(s);  break;
      case WIDTH:       s->level->width       = r(s);  break;
      case GOAL:        s->level->goal        = r(s);  break;
      case BACKGROUND:  s->level->background  = r(s);  break;
      case WATER_LEVEL: s->level->water_level = r(s);  break;
      case HEADERS_END: currently_reading_headers = 0; break;
    }
  }
  if (level_output->height * level_output->width > MAX_TILES_IN_LEVEL)  return TOO_MANY_TILES;

  // --- Tiles ---
  s->cursor *= 8;  // in bits now !!!

  size_t tidx = 0;
  while (1) { // => while tidx<height*width, està implicit
    size_t delta = 0;
    size_t delta_chunk;
    do {
      delta_chunk=r_tileinfo(s,BITS_IN_DELTA);
      delta += delta_chunk;
    } while (delta_chunk==MAX_DELTA);

    if (delta==0 && tidx>0) break;

    for (size_t di=0; di<delta; ++di) r_storetile(s,&tidx, 0); // through the delta, zero

    uint8_t kind;
    while ((kind = r_tileinfo(s,BITS_IN_KIND)))  r_storetile(s,&tidx, kind);
  }

  s->cursor = (s->cursor+4)/8; // in bytes again !!

  return OK;
}

/*
int main() {
  // gràcies stack overflow
  int fd = open("./testingfile.dog", O_RDONLY);
  printf("fd=%i\n",fd);

  struct stat s; fstat(fd, &s);
  int size = s.st_size;
  printf("size=%i\n",size);

  char* f = (char *) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (f) {
    printf("got file\n");
  } else {
    perror(NULL);

    printf("did not get file\n");
    exit(67);
  }

  uint8_t tiles_kind[2048] = { 0 };
  size_t tidxb = 0;
  for (int i = 0; i < size; i++) {
    char c;

    c = f[i];
    putchar(c);

    //  grep -o . testingfile.dog | sort -u
    if (c == '\n') continue;
    else if (c == '.') tiles_kind[tidxb++] =  0;
    else if (c == 'b') tiles_kind[tidxb++] =  1;
    else if (c == 'B') tiles_kind[tidxb++] =  2;
    else if (c == 'd') tiles_kind[tidxb++] =  3;
    else if (c == 'D') tiles_kind[tidxb++] =  4;
    else if (c == 'G') tiles_kind[tidxb++] =  5;
    else if (c == 'h') tiles_kind[tidxb++] =  6;
    else if (c == 's') tiles_kind[tidxb++] =  7;
    else if (c == 'S') tiles_kind[tidxb++] =  8;
    else if (c == 't') tiles_kind[tidxb++] =  9;
    else if (c == 'U') tiles_kind[tidxb++] = 10;
    else {
      printf("INVALID, EXITING\n");
      exit(67);
    }

  }

  // =========== AQUI COMENÇA LA PART INTERESSANT ===========

  __attribute((nonstring)) uint8_t name[13] = "Backflip Zone";

  SerdeLevel level_base = {
    .height = 13,
    .width = 26,
    .version_major = 2,
    .version_minor = 3,
    .name = name,
    .name_length = 13,
    .goal = 6,
    .background = 7,
    .water_level = 123,
    .modification_date = 1794063605,
    .tiles_kind = tiles_kind,
    .tiles_variant = NULL, // not yet done
    };

  // <-- Serialització starts here -->
  printf("Serialitzant");
  uint64_t count = 0;
  serialize(level_base, NULL, &count);
  printf("\t--- count is %lu\n", count); uint8_t* output = calloc(count, sizeof(uint8_t));
  serialize(level_base, output, NULL);
  // <-- Serialització ends here -->

  int fd_write = open("OUTPUT.hdgserde", O_RDWR | O_CREAT | O_TRUNC);
  write(fd_write, output, count); close(fd_write);


  // <-- Deserialització starts here -->
  SerdeLevel level_round_trip = { 0 };
  level_round_trip.name       = calloc(MAX_BYTES_IN_LEVELNAME, sizeof(uint8_t));
  level_round_trip.tiles_kind = calloc(MAX_TILES_IN_LEVEL, sizeof(uint8_t));
  deserialize(output, count, &level_round_trip);
  // <-- Deserialització ends here -->

  printf("Level (round trip) és: \n");
  for (size_t i=0; i < level_round_trip.height; ++i) {
    for (size_t j=0; j < level_round_trip.width; ++j) {
      uint8_t kind = level_round_trip.tiles_kind[i*level_round_trip.width+j];

      switch (kind) {
        case  0: printf("."); break;
        case  1: printf("b"); break;
        case  2: printf("B"); break;
        case  3: printf("d"); break;
        case  4: printf("D"); break;
        case  5: printf("G"); break;
        case  6: printf("h"); break;
        case  7: printf("s"); break;
        case  8: printf("S"); break;
        case  9: printf("t"); break;
        case 10: printf("U"); break;
        default: printf("WHAT??"); exit(420);
      }

    }
    printf("\n");
  }







  if (level_base.height != level_round_trip.height) printf("\tHEIGHTS DIFFERENTS: '%02x' vs '%02x'\n", level_base.height, level_round_trip.height);
  if (level_base.width != level_round_trip.width)   printf("\tWIDTHS DIFFERENTS: '%02x' vs '%02x'\n", level_base.width, level_round_trip.width);
  if (level_base.version_major != level_round_trip.version_major) printf("\tVERSION_MAJOR DIFFERENTS: '%02x' vs '%02x'\n", level_base.version_major, level_round_trip.version_major);
  if (level_base.version_minor != level_round_trip.version_minor) printf("\tVERSION_MINOR DIFFERENTS: '%02x' vs '%02x'\n", level_base.version_minor, level_round_trip.version_minor);
  if (level_base.name_length != level_round_trip.name_length) printf("\tLEVEL_NAME_LENGTH DIFFERENTS: '%zu' vs '%zu'\n", level_base.name_length, level_round_trip.name_length);
  if (memcmp(level_base.name, level_round_trip.name, level_base.name_length) != 0) printf("\tLEVEL_NAME DIFFERENTS\n");
  if (level_base.goal != level_round_trip.goal) printf("\tGOAL DIFFERENTS: '%02x' vs '%02x'\n", level_base.goal, level_round_trip.goal);
  if (level_base.background != level_round_trip.background) printf("\tBACKGROUND DIFFERENTS: '%02x' vs '%02x'\n", level_base.background, level_round_trip.background);
  if (level_base.water_level != level_round_trip.water_level) printf("\tWATER_LEVEL DIFFERENTS: '%02x' vs '%02x'\n", level_base.water_level, level_round_trip.water_level);
  if (level_base.modification_date != level_round_trip.modification_date) printf("\tMODIFICATION_DATE DIFFERENTS: '%08x' vs '%08x'\n", level_base.modification_date, level_round_trip.modification_date);
  if (memcmp(level_base.tiles_kind, level_round_trip.tiles_kind, level_base.height * level_base.width) != 0)  printf("\tTILES_KIND DIFFERENTS\n");
  for (size_t i=0; i< level_base.width*level_base.height; ++i) {
    uint8_t a = level_base.tiles_kind[i], b = level_round_trip.tiles_kind[i];
    if (a != b)  printf("\t\tCompare: i=%zu\tbase=%i\trt=%i\n", i, a,b);
  }
  //if (memcmp(level_base.tiles_variant, level_round_trip.tiles_variant, level_base.height * level_base.width) != 0) printf("TILES_VARIANT DIFFERENTS");
  ///free(level_base.tiles_kind); <- està a l'stack, no cal
  // free(level_base.name); <- està a l'stack, no cal
  free(output);
  free(level_round_trip.tiles_kind);
  free(level_round_trip.name);
}

*/

// (this is pretty oudated, a bunch of things have changed (e.g. deltas instead of (y,x) pairs))
//
// FILE = 'DOG' VERSIO HEADERS TILES VARIANTS
//
// HEADERS = NAMETAG levelname
//           GOALTAG goal
//           BACKGROUNDTAG bg
//           WATERLEVELTAG waterlevel
//           MODDATETAG modificationdate
//           HEADERENDTAG
//
//
//
// VERSIO = dos bytes: major minor
//
// goal = one byte
// bg = one byte
// waterlevel = one byte
// modificationdate = (unix timestamp en millisegons)
//
//
// levelname = <at most 64B of utf8 encoded text>
//
//
//
// TILES = (RUN)* TILES_END;
// RUN = ycoord xcoord (TILETYPE)* RUNEND
//
// TILETYPE = 1 byte
//
//
// per cada tile(=objecte): tipus (one byte), posició (dos bytes, 64x32 màx)
//
//
// variants: one byte per tile
