#ifndef LEVELOBJECTTESTENTITY_HH
#define LEVELOBJECTTESTENTITY_HH

#include "Level/LevelObject.hh"
#include "LevelObjectTest.hh"

class LevelObjectTestEntity : public LevelObjectTest {
public:
    LevelObjectTestEntity(const Rectangle& bounds, Level* level) : LevelObjectTest(bounds, level) {}

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

};

#endif