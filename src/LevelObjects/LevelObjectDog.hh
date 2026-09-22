#ifndef LEVELOBJECTDOG_HH
#define LEVELOBJECTDOG_HH

#include "LevelObject.hh"

class LevelObjectDog : public LevelObject {
public:
    LevelObjectDog(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

private:
    static constexpr float kWidth = 0.7f;
    static constexpr float kHeight = 0.5f;
};

#endif