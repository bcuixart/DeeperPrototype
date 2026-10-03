#ifndef LEVELOBJECTDOG_HH
#define LEVELOBJECTDOG_HH

#include "Level/LevelObject.hh"
#include "InputManager.hh"

enum class DogState {
    Default, Sliding, InDirt, Flung
};

class LevelObjectDog : public LevelObject {
public:
    LevelObjectDog(const Vector2& position, LevelManager* levelManager);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

    void ReceiveInput(uint8_t input);

    void DropThroughSemisolid(const Rectangle& semisolidBounds);

    void StartSliding();

	float GetInteractionVelocity() const override { return kInteractionVelocity; }

private:
    void Update_Default(const float deltaTime);
    void Update_Sliding(const float deltaTime);
    void Update_InDirt(const float deltaTime);
    void Update_Flung(const float deltaTime);

    static constexpr float kWidth = 0.7f;
    static constexpr float kHeight = 0.5f;

    static constexpr float kWalkSpeed = 10.0f;
    static constexpr float kGroundFriction = 0.1f;

    static constexpr float kInteractionVelocity = 15.0f;

    static constexpr float kSlideInitialSpeed = 0.5f;
    static constexpr float kSlideAcceleration = 20.0f;

    static constexpr float kDigHorizontalOffset = 0.01f;

    bool _inputJump{false};
    bool _inputDig{false};
    bool _inputBark{false};
    float _inputAxisX{0.0f};

    bool _inputJumpPrev{false};
    bool _inputDigPrev{false};
    bool _inputBarkPrev{false};
    float _inputAxisXPrev{0.0f};

    bool _isFacingLeft = false;

    bool _isSlidingLeft = false;

    DogState _state{DogState::Default};

    LevelManager* _levelManager;
};

#endif