#ifndef LEVELOBJECTBALLTENNIS_HH
#define LEVELOBJECTBALLTENNIS_HH

#include "LevelObject.hh"
#include "LevelObjects/LevelObjectDog.hh"

class LevelObjectBallTennis : public LevelObject {
public:
    LevelObjectBallTennis(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    void CollidedWithX(LevelObject* other, float prevVelocityX) override;
    void CollidedWithY(LevelObject* other, float prevVelocityY) override;

    void OnOverlap(LevelObject* other) override;

    HitboxType GetHitboxType() const override { return HitboxType::Dynamic; }

private:
    void KickFromDog(LevelObjectDog* dog);
    void BounceOffBall(LevelObjectBallTennis* otherBall);

    static constexpr float kWidth = 0.5f;
    static constexpr float kHeight = 0.5f;

    static constexpr float kBounciness = 0.75f;
    static constexpr float kGroundFriction = 0.99f;

    float _kickCooldown = 0.0f;
    static constexpr float kKickSpeed = 2.0f;
    static constexpr float kKickSpeedMinUpwardsVelocity = 5.0f;
    static constexpr float kKickCooldownDuration = 0.5f;
    static constexpr float kSeparationSpeed = 2.0f;
};

#endif