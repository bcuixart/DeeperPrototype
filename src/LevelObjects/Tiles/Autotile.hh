#ifndef AUTOTILE_HH
#define AUTOTILE_HH

#include <cstdint>
#include <unordered_map>

const std::unordered_map<uint8_t, int> groundAutotileMap = 
{
	// No neighbors
    {  0,  0 },

	// One cardinal
    {  2,  1 }, // N
    {  8,  2 }, // W
    { 16,  3 }, // E
    { 64,  4 }, // S

    // Two cardinals, no corners
    { 66,  5 }, // N+S
    { 24,  6 }, // W+E

	// Two cardinals, one corner
    { 10,  7 }, { 11,  8 },   // N+W,  N+W+NW
    { 18,  9 }, { 22, 10 },   // N+E,  N+E+NE
    { 72, 11 }, {104, 12 },   // S+W,  S+W+SW
    { 80, 13 }, {208, 14 },   // S+E,  S+E+SE

    // Three cardinals (N+S+W: corners NW i SW free)
    { 74, 15 }, { 75, 16 }, {106, 17 }, {107, 18 },
    // Three cardinals (N+S+E: corners NE i SE free)
    { 82, 19 }, { 86, 20 }, {210, 21 }, {214, 22 },
    // Three cardinals (N+W+E: corners NW i NE free)
    { 26, 23 }, { 27, 24 }, { 30, 25 }, { 31, 26 },
    // Three cardinals (S+W+E: corners SW i SE free)
    { 88, 27 }, {120, 28 }, {216, 29 }, {248, 30 },

    // Four cardinals (all corners free)
    { 90, 31 }, { 91, 32 }, { 94, 33 }, { 95, 34 },
    {122, 35 }, {123, 36 }, {126, 37 }, {127, 38 },
    {218, 39 }, {219, 40 }, {222, 41 }, {223, 42 },
    {250, 43 }, {251, 44 }, {254, 45 }, {255, 46 },
};

#endif