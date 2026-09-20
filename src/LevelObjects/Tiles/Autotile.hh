#ifndef AUTOTILE_HH
#define AUTOTILE_HH

#include <cstdint>
#include <unordered_set>
#include <unordered_map>

#include "LevelObjects/Tiles/LevelObjectTile.hh"

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
    { 10,  18 }, { 11,  13 }, // N+W,  N+W+NW
    { 18,  17 }, { 22, 11 },  // N+E,  N+E+NE
    { 72, 19 }, {104, 15 },   // S+W,  S+W+SW
    { 80, 16 }, {208, 9 },    // S+E,  S+E+SE

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
    { 0x0212, 58 }, // Mask 18: slope above
    { 0x020A, 59 }, // Mask 10: slope above
    { 0x4050, 57 }, // Mask 80: slope below or right
    { 0x4048, 60 }, // Mask 72: slope below or left
    { 0x40D8, 76 }, // Mask 216: slope below or left
    { 0x40DF, 77 }, // Mask 223: slope below or left
    { 0x404B, 78 }, // Mask 75: slope below or left
    { 0x407A, 79 }, // Mask 122: slope below or right
    { 0x40DA, 80 }, // Mask 218: slope below or left
    { 0x4056, 81 }, // Mask 86: slope below or right
    { 0x407F, 82 }, // Mask 127: slope below or left
    { 0x4078, 83 }, // Mask 120: slope below or right
    { 0x021B, 96 }, // Mask 27: slope above
    { 0x021E, 97 }, // Mask 30: slope above
    { 0x02FB, 98 }, // Mask 251: slope above
    { 0x02FE, 99 }, // Mask 254: slope above
    { 0x026A, 100 }, // Mask 106: slope above
    { 0x02D2, 101 }, // Mask 210: slope above

    // Tiles with 3 variations

    // mask 222: above | below or left
    { 0x02DE, 61 }, // N slope
    { 0x40DE, 62 }, // S slope
    { 0x42DE, 63 }, // N + S slope

    // mask 126: above | below or right
    { 0x027E, 64 }, // N slope
    { 0x407E, 65 }, // S slope
    { 0x427E, 66 }, // N + S slope

    // mask 219: above | below or left
    { 0x02DB, 67 }, // N slope
    { 0x40DB, 68 }, // S slope
    { 0x42DB, 69 }, // N + S slope

    // mask 123: above | below or right
    { 0x027B, 70 }, // N slope
    { 0x407B, 71 }, // S slope
    { 0x427B, 72 }, // N + S slope

    // mask 82:  above | below or right
    { 0x0252, 84 }, // N slope
    { 0x4052, 85 }, // S slope
    { 0x4252, 86 }, // N + S slope

    // mask 74:  above | below or left
    { 0x024A, 87 }, // N slope
    { 0x404A, 88 }, // S slope
    { 0x424A, 89 }, // N + S slope

    // mask 95:  left | right
    { 0x105F, 74 }, // E slope
    { 0x085F, 73 }, // W slope
    { 0x185F, 75 }, // E + W slope

    // Tiles with 7 variations

    // mask 94:
    { 0x025E, 90 }, // N
    { 0x105E, 91 }, // E
    { 0x125E, 92 }, // N+E

    // mask 91:
    { 0x025B, 93 }, // N
    { 0x085B, 94 }, // W
    { 0x0A5B, 95 }, // N+W
};

const std::unordered_map<uint16_t, uint8_t> slopeAutotileMap = 
{
    {  8, 48 }, // W
    { 16, 47 }, // E

    { 10, 55 }, { 11, 51 },  // N+W,  N+W+NW
    { 18, 54 }, { 22, 50 },  // N+E,  N+E+NE
    { 72, 56 }, {104, 52 },  // S+W,  S+W+SW
    { 80, 53 }, {208, 49 },  // S+E,  S+E+SE

    { 0x4050, 53 }, // Mask 80: slope below or right
    { 0x4048, 56 }, // Mask 72: slope below or left

    { 0x0212, 54 }, // Mask 18: slope above
    { 0x020A, 55 }, // Mask 10: slope above
};

const std::unordered_set<uint8_t> slopedSpriteIndexes =
{
    47, 48, 
    49, 50, 51, 52, 53, 54, 55, 56, 
    102, 103, 104, 105, 106, 107, 108, 109, 110, 111
};

const std::unordered_map<uint8_t, SlopeOrientation> slopeSpriteIndexToOrientation =
{
    { 47, SlopeOrientation::RisingRight },
    { 48, SlopeOrientation::RisingLeft },
    { 49, SlopeOrientation::RisingRight },
    { 50, SlopeOrientation::FallingRight },
    { 51, SlopeOrientation::FallingLeft },
    { 52, SlopeOrientation::RisingLeft },
    { 53, SlopeOrientation::RisingRight },
    { 54, SlopeOrientation::FallingRight },
    { 55, SlopeOrientation::FallingLeft },
    { 56, SlopeOrientation::RisingLeft },
    { 102, SlopeOrientation::RisingRight },
    { 103, SlopeOrientation::RisingLeft },
    { 104, SlopeOrientation::RisingRight },
    { 105, SlopeOrientation::RisingRight },
    { 106, SlopeOrientation::RisingRight },
    { 107, SlopeOrientation::FallingRight },
    { 108, SlopeOrientation::FallingLeft },
    { 109, SlopeOrientation::RisingLeft },
    { 110, SlopeOrientation::RisingLeft },
    { 111, SlopeOrientation::RisingLeft },
};

const std::unordered_set<uint8_t> slopePossibleVariationsMasks =
{
    0xD0, 0x68, 0x12, 0x0A, 0x50, 0x48
};

const std::unordered_map<uint16_t, uint8_t> slopeVariationAutotileMap =
{
    { (208 << 8) | 1, 102 },

    { (104 << 8) | 1, 103 },

    { (80 << 8) | 1, 105 },
    { (80 << 8) | 2, 104 },
    { (80 << 8) | 3, 106 },

    { (18 << 8) | 1, 107 },

    { (10 << 8) | 1, 108 },

    { (72 << 8) | 1, 110 },
    { (72 << 8) | 2, 109 },
    { (72 << 8) | 3, 111 },
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