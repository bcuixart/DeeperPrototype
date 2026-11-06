#ifndef LEVELOBJECTDOG_HH
#define LEVELOBJECTDOG_HH

#include "Level/LevelObject.hh"
#include "InputManager.hh"
#include "Network/NetworkTypes.hh"

enum class DogState {
    Default, Sliding, InTerrain, Stunned
};

class LevelObjectDog : public LevelObject {
public:
    LevelObjectDog(const Vector2& position, Level* level);

    void Update(const float deltaTime) override;
    void Render(const float deltaTime) const override;
    void RenderBounds(const float deltaTime, const Color& color) const override;

    bool NetGet(NetworkLevelObjectDynEntState& state) const override;
    void NetSet(const NetworkLevelObjectDynEntState& state) override;

    void CollidedWithX(LevelObject* other, float prevVelocityX) override;
    void CollidedWithY(LevelObject* other, float prevVelocityY) override;

    void OnDug(LevelObject* digger) override;

    HitboxType GetHitboxType() const override { return HitboxType::Entity; }

    void ReceiveInput(uint8_t input);
    void SetSlot(uint8_t slot) { _slot = slot; }
    uint8_t GetSlot() const { return _slot; }
    uint8_t GetInteractionCauseSlot() const override { return _interactionCauseSlot; }
    uint8_t GetInteractionDepth() const { return _interactionDepth; }

    void OnForceField(const LevelObject* field) override;

    void SetGone() { _isGone = true; }

    void OnEnteredTerrain(int tileX, int tileY, float velX, float velY, bool enteredByDigging) override;
    bool IsInTerrain() const override { return _state == DogState::InTerrain; }

    void SetDogType(uint8_t dogType) { _dogType = dogType % kNumDogTypes; }

    void DropThroughSemisolid(const Rectangle& semisolidBounds);

    void StartSliding();

    void StunAddVelocity(const LevelObject* stunner, const float velX, const float velY, const float stunTime);
    void StunSetVelocity(const LevelObject* stunner, const float velX, const float velY, const float stunTime);

	float GetInteractionVelocity() const override { return kInteractionVelocity; }

private:
    void Update_Default(const float deltaTime);
    void Update_Sliding(const float deltaTime);
    void Update_InTerrain(const float deltaTime);
    void Update_Stunned(const float deltaTime);

    void StartJump();

    void Stun(const LevelObject* stunner, const float stunTime);

    static constexpr float kWidth = 0.7f;
    static constexpr float kHeight = 0.625f;

    static constexpr float kWalkSpeed = 10.0f;
    static constexpr float kGroundFriction = 0.1f;

    static constexpr float kJumpVelocity = -10.0f;
    static constexpr float kJumpHoldTime = 0.25f;
    static constexpr float kJumpReleasedVelocityMultiplier = 0.75f;

    static constexpr float kBarkJumpVelocity = -7.5f;
    static constexpr float kBarkCooldown = 0.1f;

    static constexpr float kInteractionVelocity = 13.0f;

    static constexpr float kSlideInitialSpeed = 0.5f;
    static constexpr float kSlideAcceleration = 20.0f;

    static constexpr float kDigHorizontalOffset = 0.01f;

    static constexpr float kCoyoteTime = 0.1f;
    float _coyoteTimer{0.0f};

    static constexpr float kJumpInputBufferTime = 0.1f;
    float _jumpInputBufferTimer{0.0f};

    static constexpr float kInputTerrainMoveSpeed = 10.0f;

    static constexpr float kStunnedBounciness = 0.25f;
    static constexpr float kStunnedBounceMinVelocityY = 0.5f;
    static constexpr float kStunMaxTime = 5.0f;
    float _stunTimer{0.0f};

    bool _inputJump{false};
    bool _inputDig{false};
    bool _inputBark{false};
    float _inputAxisX{0.0f};

    bool _inputJumpPrev{false};
    bool _inputDigPrev{false};
    bool _inputBarkPrev{false};
    float _inputAxisXPrev{0.0f};

    DogState _state{DogState::Default};
    bool _isGone{false};

    bool _isFacingLeft = false;
    bool _isJumping = false;
    float _jumpTimer{0.0f};

    bool _canDoBarkJump{false};
    float _barkCooldownTimer{0.0f};

    bool _isSlidingLeft = false;

    uint8_t _interactionCauseSlot{kInteractionNoCause};
    uint8_t _interactionDepth{0};

    uint8_t _slot{0};
    uint8_t _dogType{0};

    Level* _level;
};

#endif