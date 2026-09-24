#ifndef LEVELOBJECTBALLTENNIS_HH
#define LEVELOBJECTBALLTENNIS_HH

#include "LevelObject.hh"

class LevelObjectBallTennis : public LevelObject {
public:
    LevelObjectBallTennis(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    void CollidedWithX(LevelObject* other, float prevVelocityX) override;
    void CollidedWithY(LevelObject* other, float prevVelocityY) override;

    HitboxType GetHitboxType() const override { return HitboxType::Dynamic; }

private:
    static constexpr float kWidth = 0.5f;
    static constexpr float kHeight = 0.5f;

    static constexpr float kBounciness = 0.75f;
    static constexpr float kGroundFriction = 0.99f;
};

#endif