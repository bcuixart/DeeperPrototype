#ifndef LEVELOBJECTBONE_HH
#define LEVELOBJECTBONE_HH

#include "Level/LevelObject.hh"
#include "Level/LevelObjects/LevelObjectDog.hh"

class LevelObjectBone : public LevelObject {
public:
    LevelObjectBone(const Vector2& position);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    bool NetGet(NetworkLevelObjectDynEntState& state) const override;
    void NetSet(const NetworkLevelObjectDynEntState& state) override;

    HitboxType GetHitboxType() const override { return HitboxType::Trigger; }

    void OnOverlap(LevelObject* other) override;

    GrabbableType GetGrabbableType() const override { return GrabbableType::Bone; }

private:
    static constexpr float kWidth = 0.875f;
    static constexpr float kHeight = 0.4375f;

    bool _wasGrabbed = false;
};

#endif