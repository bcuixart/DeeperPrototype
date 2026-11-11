#ifndef LEVELOBJECTTESTDYNAMIC_HH
#define LEVELOBJECTTESTDYNAMIC_HH

#include "Level/LevelObject.hh"
#include "LevelObjectTest.hh"

class LevelObjectTestDynamic : public LevelObjectTest {
public:
    LevelObjectTestDynamic(const Rectangle& bounds, Level* level) : LevelObjectTest(bounds, level) {}

    HitboxType GetHitboxType() const override { return HitboxType::Dynamic; }
};

#endif