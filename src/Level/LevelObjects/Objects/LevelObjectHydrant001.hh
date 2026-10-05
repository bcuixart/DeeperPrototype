#ifndef LEVELOBJECTHYDRANT001_HH
#define LEVELOBJECTHYDRANT001_HH

#include "Level/LevelObject.hh"

class LevelObjectHydrant001Water;

class LevelObjectHydrant001 : public LevelObject {
public:
    LevelObjectHydrant001(const Vector2& position, LevelObjectHydrant001Water* waterLeft, LevelObjectHydrant001Water* waterRight);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::SemisolidPartial; }

    void CollidedWithX(LevelObject* other, float prevVelocityX) override;
    void CollidedWithY(LevelObject* other, float prevVelocityY) override;

    void OnDug(LevelObject* digger) override;

private:
    void Activate();
    void Deactivate();

    static constexpr float kWidth = 0.5f;
    static constexpr float kHeight = 1.25f;

    static constexpr float kActiveDuration = 3.0f;

    LevelObjectHydrant001Water* _waterLeft;
    LevelObjectHydrant001Water* _waterRight;

    float _activeTimer = 0.0f;

    bool _isActive = false;
};


class LevelObjectHydrant001Water : public LevelObject {
public:
    LevelObjectHydrant001Water(const Vector2& position, bool isLeft);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;
    void RenderBounds(const float deltaTime, const Color& color) const override;

    HitboxType GetHitboxType() const override { return HitboxType::ForceField; }

    void Activate();
    void Deactivate();

    bool GetForceFieldIsActive() const override { return _isActive; }
    Vector2 GetForceFieldForce() const override;

private:
    static constexpr float kWidth = 1.3125f;
    static constexpr float kHeight = 1.1875f;

    static constexpr float kHorizontalOffset = 0.4375f;
    static constexpr float kVerticalOffset = 0.0625f;

    static constexpr float kForceFieldDirectionX = 100.0f;
    static constexpr float kForceFieldDirectionY = -25.0f;

    bool _isLeft;

    bool _isActive = false;
};
#endif