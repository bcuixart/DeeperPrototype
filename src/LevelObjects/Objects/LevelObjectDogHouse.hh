#ifndef LEVELOBJECTDOGHOUSE_HH
#define LEVELOBJECTDOGHOUSE_HH

#include "LevelObject.hh"

class LevelObjectDogHouse : public LevelObject {
public:
    LevelObjectDogHouse(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Trigger; }

private:
    static constexpr float kWidth = 0.75f;
    static constexpr float kHeight = 0.8f;
};

#endif