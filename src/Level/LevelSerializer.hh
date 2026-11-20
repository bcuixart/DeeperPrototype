#ifndef LEVELSERIALIZER_HH
#define LEVELSERIALIZER_HH

/**
 * @file main.c
 * @brief (De)serialization for Hot Dog Fest
 * @author Amat Gil
 * @version 1
 *
 * The format is optimized for hand-readability, ease of implementation,
 * and compression capabilities. Levels are small enough
 * that hyper-optimizing's returns are immediately diminishing, so the
 * only 'compression' is:
 * - tiles are encoded in runs, to avoid emitting redundant position information
 * - variants, since they're relatively rare, are encoded at the end (and only
 *   for the ones that differ from the established background).

 * Notably, headers are not compressed (there's no predetermined ordered,
 * etc). It's all KEY VALUE KEY VALUE .. HEADERS_END.
 *
 **/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	uint8_t  height;
	uint8_t  width;
	uint8_t  version_major;
	uint8_t  version_minor;
	uint8_t* name;
	size_t   name_length; // confio cegament que sigui correcte (si és <64)
	uint8_t  goal;
	uint8_t  background;
	uint8_t  water_level;
	uint32_t modification_date; // unix timestamp (LSb first)
	uint8_t* tiles_kind;        // ha de tindre mida height * width
	uint8_t* tiles_variant;     // ha de tindre mida height * width
} SerdeLevel;

typedef enum {
	// -- Dels dos --
	LEVEL_NAME_TOO_LONG, // més bytes que MAX_BYTES_IN_LEVELNAME
	TOO_MANY_TILES,      // més tiles que MAX_TILES_IN_LEVEL
	// -- Els de SER --
	OK,
	// -- Els de DE --
	NO_MAGIC_BYTES_FOUND,
	INVALID_FORMAT, // hauria de ser més especific
	UNEXPECTED_EOF,
	OUTPUT_LEVEL_PTR_WAS_NULL,
} HDF_SERDE_RESULT;

const uint64_t MAX_BYTES_IN_LEVELNAME = 64;

/*
 * `output` must be "large enough". To find out which size that is, use
 * a non-null `byte_count`.
 * - if NULL:     data is written into `output`
 * - if not NULL: `output` is not written to, instead byte_count returns how
 *                many bytes are written to output total
 *
 * Example usage:
 * ```
 * SerdeLevel level = ... ;
 * uint64_t count = 0; serialize(level, NULL, &count);
 * uint8_t* output = calloc(count, sizeof(uint8_t));
 * serialize(level, output, NULL);
 * ```
 */
HDF_SERDE_RESULT serialize(SerdeLevel level, uint8_t* output, uint64_t* byte_count);


/*
 * `level_output` must be a non-null ptr where:
 *    `level_output->name_length` must be a pointer/array with MAX_BYTES_IN_LEVELNAME available
 *    `level_output->tiles_kind`  must be a pointer/array with MAX_TILES_IN_LEVEL bytes available
 *
 * Example usage:
 * ```
 * SerdeLevel level = { 0 };
 * level.name       = calloc(MAX_BYTES_IN_LEVELNAME, sizeof(uint8_t));
 * level.tiles_kind = calloc(MAX_TILES_IN_LEVEL, sizeof(uint8_t));
 * deserialize(output, count, &level_round_trip);
 * ```
*/
HDF_SERDE_RESULT deserialize(uint8_t* input, size_t input_length, SerdeLevel* level_output);

#endif