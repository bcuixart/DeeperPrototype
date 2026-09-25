#ifndef LEVELOBJECTBENCH001_HH
#define LEVELOBJECTBENCH001_HH

#include "LevelObject.hh"
#include "LevelObjects/LevelObjectDog.hh"

class LevelObjectBench001 : public LevelObject {
public:
    LevelObjectBench001(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::SemisolidTotal; }

    void OnDug(LevelObject* digger) override;

private:
    static constexpr float kWidth = 1.5f;
    static constexpr float kHeight = 0.5f;
};

#endif