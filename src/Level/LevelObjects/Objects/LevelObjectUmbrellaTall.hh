#ifndef LEVELOBJECTUMBRELLATALL_HH
#define LEVELOBJECTUMBRELLATALL_HH

#include "Level/LevelObject.hh"

class LevelObjectUmbrellaTall : public LevelObject {
public:
    LevelObjectUmbrellaTall(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Trampoline; }

private:
    static constexpr float kWidth = 1.25f;
    static constexpr float kHeight = 0.2f;

    // The top the umbrella must not be alligned with the top of a solid tile
    // to prevent weird collision priority resolutions
    static constexpr float kTrampolineHeight = 2.5f;
};

#endif