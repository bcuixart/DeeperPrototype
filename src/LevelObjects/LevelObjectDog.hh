#ifndef LEVELOBJECTDOG_HH
#define LEVELOBJECTDOG_HH

#include "LevelObject.hh"

class LevelObjectDog : public LevelObject {
public:
    LevelObjectDog(const Vector2& position, LevelManager& level);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    bool IsGrounded() const { return _isGrounded; }

private:
    void ApplyGravity(const float deltaTime);
    void MoveAndCollide(const float deltaTime);
    void ResolveTilesX();
    void ResolveTilesY();

    LevelManager& _level;

    Vector2 _velocity = { 0, 0 };
    bool _isGrounded = false;

    static constexpr float kGravity = 25.0f;
    static constexpr float kMaxFallSpeed = 20.0f;
    static constexpr float kWidth = 0.95f;
    static constexpr float kHeight = 0.5f;
};

#endif