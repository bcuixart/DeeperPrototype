#ifndef LEVELOBJECTTEST_HH
#define LEVELOBJECTTEST_HH

#include "Level/LevelObject.hh"

class LevelObjectTest : public LevelObject {
public:
    LevelObjectTest(const Rectangle& bounds, Level* level);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

protected:
    Level* _level{ nullptr };
};

#endif