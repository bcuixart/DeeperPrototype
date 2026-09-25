#ifndef LEVELOBJECTDOG_HH
#define LEVELOBJECTDOG_HH

#include "LevelObject.hh"

enum class DogState {
    Default, SlidingLeft, SlidingRight, InDirt, Flung
};

class LevelObjectDog : public LevelObject {
public:
    LevelObjectDog(const Vector2& position, LevelManager* levelManager);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

    void DropThroughSemisolid(const Rectangle& semisolidBounds);

    void StartSlidingRight();
    void StartSlidingLeft();

private:
    void Update_Default(const float deltaTime);
    void Update_Sliding(const float deltaTime, bool isSlidingLeft);
    void Update_InDirt(const float deltaTime);
    void Update_Flung(const float deltaTime);

    static constexpr float kWidth = 0.7f;
    static constexpr float kHeight = 0.5f;

    static constexpr float kWalkSpeed = 10.0f;

    static constexpr float kSlideInitialSpeed = 0.5f;
    static constexpr float kSlideAcceleration = 10.0f;

    static constexpr float kDigHorizontalOffset = 0.01f;

    DogState _state{DogState::Default};

    LevelManager* _levelManager;
};

#endif