#ifndef LEVELOBJECTDOGHOUSE_HH
#define LEVELOBJECTDOGHOUSE_HH

#include "Level/LevelObject.hh"
#include "Level/LevelObjects/LevelObjectDog.hh"

class LevelObjectDogHouse : public LevelObject {
public:
    LevelObjectDogHouse(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Trigger; }

    void OnOverlap(LevelObject* other) override;

    void SetDogSlotMask(uint8_t mask) { _dogSlotsMask = mask; }
    void AddDogSlot(uint8_t slot) { if (slot < MAX_PLAYERS) _dogSlotsMask |= (1u << slot); }

private:
    static constexpr float kWidth = 0.75f;
    static constexpr float kHeight = 0.8f;

    uint8_t _dogSlotsMask{0};
};

#endif