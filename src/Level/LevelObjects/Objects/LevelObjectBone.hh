#ifndef LEVELOBJECTBONE_HH
#define LEVELOBJECTBONE_HH

#include "Level/LevelObject.hh"
#include "Level/LevelObjects/LevelObjectDog.hh"

class LevelObjectBone : public LevelObject {
public:
    LevelObjectBone(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Trigger; }

private:
    static constexpr float kWidth = 0.875f;
    static constexpr float kHeight = 0.4375f;
};

#endif