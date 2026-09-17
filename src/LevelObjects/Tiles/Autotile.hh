#ifndef AUTOTILE_HH
#define AUTOTILE_HH

#include <cstdint>
#include <unordered_set>
#include <unordered_map>

const std::unordered_map<uint16_t, uint8_t> groundAutotileMap = 
{
    // ---=== BASE TILES === ---
	// No neighbors
    {  0,  1 },

	// One cardinal
    {  2,  7 }, // N
    {  8,  4 }, // W
    { 16,  2 }, // E
    { 64,  5 }, // S

    // Two cardinals, no corners
    { 66,  6 }, // N+S
    { 24,  3 }, // W+E

	// Two cardinals, one corner
    { 10,  18 }, { 11,  13 },   // N+W,  N+W+NW
    { 18,  17 }, { 22, 11 },   // N+E,  N+E+NE
    { 72, 19 }, {104, 15 },   // S+W,  S+W+SW
    { 80, 16 }, {208, 9 },   // S+E,  S+E+SE

    // Three cardinals (N+S+W: corners NW i SW free)
    { 74, 36 }, { 75, 28 }, {106, 45 }, {107, 14 },
    // Three cardinals (N+S+E: corners NE i SE free)
    { 82, 35 }, { 86, 32 }, {210, 46 }, {214, 10 },
    // Three cardinals (N+W+E: corners NW i NE free)
    { 26, 30 }, { 27, 41 }, { 30, 42 }, { 31, 12 },
    // Three cardinals (S+W+E: corners SW i SE free)
    { 88, 38 }, {120, 34 }, {216, 26 }, {248, 8 },

    // Four cardinals (all corners free)
    { 90, 37 }, { 91, 40 }, { 94, 39 }, { 95, 25 },
    {122, 29 }, {123, 23 }, {126, 21 }, {127, 33 },
    {218, 31 }, {219, 22 }, {222, 20 }, {223, 27 },
    {250, 24 }, {251, 43 }, {254, 44 }, {255, 0 },

    // ---=== SLOPE VARIATIONS ===---
    // Tiles with 1 variation
    { 0x0212, 126 }, // Mask 18: slope above
    { 0x020A, 126 }, // Mask 10: slope above
    { 0x4050, 126 }, // Mask 80: slope below or right
    { 0x4048, 126 }, // Mask 72: slope below or left
    { 0x40D8, 126 }, // Mask 216: slope below or left
    { 0x40DF, 126 }, // Mask 223: slope below or left
    { 0x404B, 126 }, // Mask 75: slope below or left
    { 0x407A, 126 }, // Mask 122: slope below or right
    { 0x40DA, 126 }, // Mask 218: slope below or left
    { 0x4056, 126 }, // Mask 86: slope below or right
    { 0x407F, 126 }, // Mask 127: slope below or left
    { 0x4078, 126 }, // Mask 120: slope below or right
    { 0x021B, 126 }, // Mask 27: slope above
    { 0x021E, 126 }, // Mask 30: slope above
    { 0x02FB, 126 }, // Mask 251: slope above
    { 0x02FE, 126 }, // Mask 254: slope above
    { 0x026A, 126 }, // Mask 106: slope above
    { 0x02D2, 126 }, // Mask 210: slope above

    // Tiles with 3 variations

    // mask 222: above | below or left
    { 0x02DE, 126 }, // N slope
    { 0x40DE, 126 }, // S slope
    { 0x42DE, 126 }, // N + S slope

    // mask 126: above | below or right
    { 0x027E, 126 }, // N slope
    { 0x407E, 126 }, // S slope
    { 0x427E, 126 }, // N + S slope

    // mask 219: above | below or left
    { 0x02DB, 126 }, // N slope
    { 0x40DB, 126 }, // S slope
    { 0x42DB, 126 }, // N + S slope

    // mask 123: above | below or right
    { 0x027B, 126 }, // N slope
    { 0x407B, 126 }, // S slope
    { 0x427B, 126 }, // N + S slope

    // mask 82:  above | below or right
    { 0x0252, 126 }, // N slope
    { 0x4052, 126 }, // S slope
    { 0x4252, 126 }, // N + S slope

    // mask 74:  above | below or left
    { 0x024A, 126 }, // N slope
    { 0x404A, 126 }, // S slope
    { 0x424A, 126 }, // N + S slope

    // mask 95:  left | right
    { 0x105F, 126 }, // E slope
    { 0x085F, 126 }, // W slope
    { 0x185F, 126 }, // E + W slope

    // mask 90:  left | right
    { 0x105A, 126 }, // E slope
    { 0x085A, 126 }, // W slope
    { 0x185A, 126 }, // E + W slope

    // mask 88:  left | right
    { 0x1058, 126 }, // E slope
    { 0x0858, 126 }, // W slope
    { 0x1858, 126 }, // E + W slope

    // Tiles with 7 variations

    // mask 94:
    { 0x025E, 126 }, // N
    { 0x085E, 126 }, // W
    { 0x105E, 126 }, // E
    { 0x0A5E, 126 }, // N+W
    { 0x125E, 126 }, // N+E
    { 0x185E, 126 }, // W+E
    { 0x1A5E, 126 }, // N+W+E

    // mask 91:
    { 0x025B, 126 }, // N
    { 0x085B, 126 }, // W
    { 0x105B, 126 }, // E
    { 0x0A5B, 126 }, // N+W
    { 0x125B, 126 }, // N+E
    { 0x185B, 126 }, // W+E
    { 0x1A5B, 126 }, // N+W+E
};

const std::unordered_set<uint8_t> slopedTiles =
{
    2, 4,
    9, 11, 13, 15, 16, 17, 18, 19
};

const std::unordered_map<uint8_t, uint8_t> slopesSpriteMap = 
{
    {  2,  47 },
    {  4,  48 },
    {  9,  49 },
    {  11,  50 },
    {  13,  51 },
    {  15,  52 },
    {  16,  53 },
    {  17,  54 },
    {  18,  55 },
    {  19,  56 },
};

const std::unordered_map<uint8_t, uint8_t> bridgeAutotileMap = 
{
    // No neighbors
    {  0,  0 },

    // One neighbors
    {  1,  3 }, // W
    {  2,  1 }, // E

    // Two neighbors
    {  3,  2 }, // W+E
};

#endif