#ifndef LEVELOBJECTHYDRANT001_HH
#define LEVELOBJECTHYDRANT001_HH

#include "LevelObject.hh"

class LevelObjectHydrant001 : public LevelObject {
public:
    LevelObjectHydrant001(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::SemisolidPartial; }

private:
    static constexpr float kWidth = 0.5f;
    static constexpr float kHeight = 1.25f;
};

#endif