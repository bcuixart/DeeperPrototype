#include "LevelObjectDog.hh"
#include "Level/Level.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position, Level* level)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
    _level = level;
}

void LevelObjectDog::Update(const float deltaTime) 
{
    _coyoteTimer = _isGrounded ? kCoyoteTime : std::max(0.0f, _coyoteTimer - deltaTime);
    const bool jumpPressedThisFrame = _inputJump && !_inputJumpPrev;
    _jumpInputBufferTimer = jumpPressedThisFrame ? kJumpInputBufferTime : std::max(0.0f, _jumpInputBufferTimer - deltaTime);
    _barkCooldownTimer = std::max(0.0f, _barkCooldownTimer - deltaTime);
    if (_isGrounded) _canDoBarkJump = true;

    switch (_state)
    {
        case DogState::Default:
            Update_Default(deltaTime);
            break;
        case DogState::Sliding:
            Update_Sliding(deltaTime);
            break;
        case DogState::InTerrain:
            Update_InTerrain(deltaTime);
            break;
        case DogState::Stunned:
            Update_Stunned(deltaTime);
            break;
    }

    _inputJumpPrev = _inputJump;
    _inputDigPrev = _inputDig;
    _inputBarkPrev = _inputBark;
    _inputAxisXPrev = _inputAxisX;
}

void LevelObjectDog::Update_Default(const float deltaTime)
{
    // Since we're in control, the interactions are in our name
    _interactionCauseSlot = _slot;
    _interactionDepth = 0;

    // Coyote time accounts for grounded check, input buffer accounts for input
    if (_coyoteTimer > 0.0f && _jumpInputBufferTimer > 0.0f && !_isJumping)
    {
        StartJump();
    }
    else if (_isJumping)
    {
        _jumpTimer -= deltaTime;
        
        // Stop jump if button is released, timer is over, or bumped into ceiling
        if (GetVelocityY() >= 0.0f) _isJumping = false;
        else if (!_inputJump || _jumpTimer <= 0.0f)
        {
            _isJumping = false;
            SetVelocityY(GetVelocityY() * kJumpReleasedVelocityMultiplier);
        }
        else SetVelocityY(kJumpVelocity);
    }

    // Bark and bark jump mechanic
    if (_inputBark && !_inputBarkPrev && _barkCooldownTimer <= 0.0f)
    {
        if (_canDoBarkJump)
        {
            _canDoBarkJump = false;

            if (_isJumping) _isJumping = false;
            SetVelocityY(kBarkJumpVelocity);
        }   

        _barkCooldownTimer = kBarkCooldown;
        _level->BarkAt(this);
    }

    float currentVelocityX = GetVelocityX();
    float desiredVelocityX = kWalkSpeed * _inputAxisX;

    // Update facing direction based on input
    if (_inputAxisX > 0.0f) _isFacingLeft = false;
    else if (_inputAxisX < 0.0f) _isFacingLeft = true;

    if (desiredVelocityX != 0.0f)
    {
        // If below walk speed or moving in opposite direction, set velocity to desired velocity
        const bool belowOrOpposite = fabsf(currentVelocityX) < fabsf(desiredVelocityX)
                                    || (currentVelocityX >= 0.0f) != (desiredVelocityX >= 0.0f);
        if (belowOrOpposite) SetVelocityX(desiredVelocityX);
        // Apply friction if no input and grounded, but don't stop if moving faster than walk speed
        else if (_isGrounded) SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }
    // Stop horizontal movement if no input and below walk speed
    else if (fabsf(currentVelocityX) <= kWalkSpeed && (_inputAxisX == 0.0f && _inputAxisXPrev != 0.0f))
    {
        SetVelocityX(0.0f);
    }
    // Apply friction if no input and grounded
    else if (_isGrounded)
    {
        SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }

    // Dig mechanic
    if (_inputDig && !_inputDigPrev) 
    {
        if (_isGrounded)
        {
            _level->DigAt(this, { _bounds.x + kDigHorizontalOffset, _bounds.y + _bounds.height }, 
                                { _bounds.x + _bounds.width - kDigHorizontalOffset, _bounds.y + _bounds.height });
        }
    }
}

void LevelObjectDog::Update_Sliding(const float deltaTime)
{
    // Since we're in control, the interactions are in our name
    _interactionCauseSlot = _slot;
    _interactionDepth = 0;

    // Stop sliding if you're grounded but not on a slope
    if (_isGrounded && _surfaceTypeStandingOn != SurfaceType::SlopedLeftGround && _surfaceTypeStandingOn != SurfaceType::SlopedRightGround) { _state = DogState::Default; return; }

    // Stop sliding if you're not grounded and not falling (you keep sliding if you're falling)
    if (!_isGrounded && GetVelocityY() < 0.0f) { _state = DogState::Default; return; }

    // Stop sliding if you hit an obstacle that stops your horizontal movement
    if (GetVelocityX() == 0.0f) { _state = DogState::Default; return; }

	_isFacingLeft = (GetVelocityX() < 0.0f);

    // Coyote time accounts for grounded check, input buffer accounts for input
    if (_coyoteTimer > 0.0f && _jumpInputBufferTimer > 0.0f)
    {
        StartJump();

        _state = DogState::Default;
        return;
    }

    // Determine sliding direction based on the slope type, you keep moving the same direction if not grounded
    if (_isGrounded) _isSlidingLeft = _surfaceTypeStandingOn == SurfaceType::SlopedRightGround;

    // Move in a 45-degree angle downwards while sliding
    AddVelocityX(_isSlidingLeft ? -kSlideAcceleration * deltaTime : kSlideAcceleration * deltaTime);
    if (_isGrounded) SetVelocityY(fabsf(GetVelocityX()));
}

void LevelObjectDog::Update_InTerrain(const float deltaTime)
{
    // Since we're in control, the interactions are in our name
    _interactionCauseSlot = _slot;
    _interactionDepth = 0;

    _coyoteTimer = 0.0f;
    _jumpInputBufferTimer = 0.0f;
    _isJumping = false;
    _jumpTimer = 0.0f;
    _canDoBarkJump = true;
    _isGrounded = false;

    // We might be stunned while in terrain
    _stunTimer = std::min(_stunTimer, kStunMaxTime);
    _stunTimer = std::max(0.0f, _stunTimer - deltaTime);

    // Check if the dog is centered on the target tile
    bool centeredOnTargetTile = _level->IsObjectCenteredOnTerrainTile(this, _terrainTargetTileX, _terrainTargetTileY);

    if (centeredOnTargetTile)
    {
        // Reset desired move speed to not override possible inertia
        _terrainDesiredMoveSpeed = 0.0f;

        // Check if we exited terrain
        if (!_level->IsTileTerrain(_terrainTargetTileX, _terrainTargetTileY))
        {
            _state = (_stunTimer > 0.0f) ? DogState::Stunned : DogState::Default;
            return;
        }

        float velX = GetVelocityX();
        float velY = GetVelocityY();
        // If the velocities are not zero, it means inertia is affecting us.
        // No control for us, then!
        if (velX != 0.0f || velY != 0.0f) return;
        if (_stunTimer > 0.0f) return;

        int8_t inputDeltaX = 0;
        int8_t inputDeltaY = 0;

        // If no inertia, move because of input
        if (_inputAxisX > 0.0f) { _isFacingLeft = false; inputDeltaX = 1; }
        else if (_inputAxisX < 0.0f) { _isFacingLeft = true; inputDeltaX = -1; }

        if (_inputDig) inputDeltaY = 1;

        // If no input, do nothing
        if (inputDeltaX == 0 && inputDeltaY == 0) return;

        // Otherwise, check if the movement is possible
        int8_t deltaX, deltaY;
        _level->GetTerrainMovementDelta(_terrainTargetTileX, _terrainTargetTileY, _terrainTargetTileX + inputDeltaX, _terrainTargetTileY + inputDeltaY, deltaX, deltaY);

        if (deltaX == 0 && deltaY == 0) return;

        // If the movement is possible, set the new target tile and desired move speed
        _terrainTargetTileX = _terrainTargetTileX + deltaX;
        _terrainTargetTileY = _terrainTargetTileY + deltaY;
        _terrainDesiredMoveSpeed = kInputTerrainMoveSpeed;
    }

    // Otherwise, if the dog is not centered on the target tile, it will continue moving towards 
    // Whatever happens next, it shall be determined upon reaching the next tile
}

void LevelObjectDog::Update_Stunned(const float deltaTime)
{
    _coyoteTimer = 0.0f;
    _jumpInputBufferTimer = 0.0f;
    _isJumping = false;
    _jumpTimer = 0.0f;
    _canDoBarkJump = false;

    _stunTimer = std::min(_stunTimer, kStunMaxTime);
    _stunTimer = std::max(0.0f, _stunTimer - deltaTime);

    // Apply friction if grounded
    if (_isGrounded) SetVelocityX(GetVelocityX() * powf(kGroundFriction, deltaTime));

    if (GetVelocityX() > 0.0f) _isFacingLeft = false;
    else if (GetVelocityX() < 0.0f) _isFacingLeft = true;

    //printf("Dog stunned, timer: %f, grounded: %d, velocityX: %f\n", _stunTimer, _isGrounded, GetVelocityX());

    // Return to default state when stunned time is over, landed and moving at walk speed or less
    if (fabsf(GetVelocityX()) <= kWalkSpeed && _stunTimer <= 0.0f && _isGrounded) { _state = DogState::Default; return; }
}

void LevelObjectDog::StartJump()
{
    if (_isJumping) return;

    SetVelocityY(kJumpVelocity);

    _isJumping = true;
    _jumpTimer = kJumpHoldTime;

    _coyoteTimer = 0.0f;
    _jumpInputBufferTimer = 0.0f;
}

void LevelObjectDog::StunAddVelocity(const LevelObject* stunner, const float velX, const float velY, const float stunTime)
{
    if (stunner == nullptr) return;
    if (stunner == this) return;

    if (_state == DogState::InTerrain) return;

    AddVelocityX(velX);
    AddVelocityY(velY);

    Stun(stunner, stunTime);
}

void LevelObjectDog::StunSetVelocity(const LevelObject* stunner, const float velX, const float velY, const float stunTime)
{
    if (stunner == nullptr) return;
    if (stunner == this) return;

    if (_state == DogState::InTerrain) return;

    SetVelocityX(velX);
    SetVelocityY(velY);

    Stun(stunner, stunTime);
}

void LevelObjectDog::Stun(const LevelObject* stunner, const float stunTime)
{
    if (stunner == nullptr) return;
    
    if (_state == DogState::InTerrain) return;

    _state = DogState::Stunned;

    if (stunTime > _stunTimer)
    {
        _stunTimer = stunTime;
    }

    // Set the interaction cause and depth based on the stunner
    // A very good question here is whether or not it can change the interaction cause and depth if it is already stunned by another stunner
    // and what happens when stunned by something with no cause (ball affected by gravity): should those points count as the dog's or nobody's?
    if (stunner->GetInteractionCauseSlot() == kInteractionNoCause) return;

    _interactionCauseSlot = stunner->GetInteractionCauseSlot();
    _interactionDepth = NextInteractionDepth(stunner->GetInteractionDepth());
}

void LevelObjectDog::ReceiveInput(uint8_t input)
{
    _inputJump = (input & BITMASK_LEVEL_JUMP) != 0;
    _inputDig = (input & BITMASK_LEVEL_DIG) != 0;
    _inputBark = (input & BITMASK_LEVEL_BARK) != 0;

    uint8_t axisX = (input & BITMASK_LEVEL_AXIS_X) >> 3;

    constexpr int kAxisXCenter = 16;
    int axisXSigned = static_cast<int>(axisX) - kAxisXCenter;
    _inputAxisX = (axisXSigned >= 0) ? axisXSigned / 15.0f : axisXSigned / 16.0f;
}

void LevelObjectDog::DropThroughSemisolid(const Rectangle& semisolidBounds)
{
    if (_state != DogState::Default) return;
    if (!_isGrounded) return;

    constexpr float kDropThroughMargin = 0.05f;
	_bounds.y += kDropThroughMargin;
}

void LevelObjectDog::StartSliding()
{
    if (_state != DogState::Default) return;
    if (!_isGrounded || (_surfaceTypeStandingOn != SurfaceType::SlopedLeftGround && _surfaceTypeStandingOn != SurfaceType::SlopedRightGround)) return;

    bool startingOnLeftSlope = _surfaceTypeStandingOn == SurfaceType::SlopedRightGround;

    _state = DogState::Sliding;
    _isJumping = false;
    if (GetVelocityX() == 0.0f) SetVelocityX(startingOnLeftSlope ? -kSlideInitialSpeed : kSlideInitialSpeed);
    if (GetVelocityY() == 0.0f) SetVelocityY(kSlideInitialSpeed);
}

void LevelObjectDog::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId((uint8_t)TextureId::DogTest_Long + _dogType));

    const float spriteSize = 2.0f;

    Rectangle src = { 0.0f, 0.0f, _isFacingLeft ? -32.0f : 32.0f, 32.0f };
    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f + _renderOffset.x,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f + _renderOffset.y,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, _isGone ? Color{ 110, 110, 110, 170 } : WHITE);
}

void LevelObjectDog::RenderBounds(const float deltaTime, const Color& color) const
{
    LevelObject::RenderBounds(deltaTime, color);

    DrawCircleV({ _bounds.x + kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
    DrawCircleV({ _bounds.x + _bounds.width - kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
}

bool LevelObjectDog::NetGet(NetworkLevelObjectDynEntState& s) const 
{
    s.x = _bounds.x;  s.y = _bounds.y;
    s.vx = _velocity.x; s.vy = _velocity.y;
    s.mode = static_cast<uint8_t>(_state);
    s.flags = (_isJumping ? 1 : 0) | (_isSlidingLeft ? 2 : 0) | (_isFacingLeft ? 4 : 0)
        | (_inputJumpPrev ? 8 : 0) | (_inputDigPrev ? 16 : 0) | (_inputBarkPrev ? 32 : 0)
        | (_isGrounded ? 64 : 0) | (_canDoBarkJump ? 128 : 0);
    s.flags2 = 0;
    s.aux = static_cast<uint8_t>(_surfaceTypeStandingOn);
    s.t0 = _jumpTimer; s.t1 = _coyoteTimer; s.t2 = _jumpInputBufferTimer;
    s.t3 = _inputAxisXPrev;
    s.t4 = _stunTimer;
    s.t5 = _barkCooldownTimer;
    s.t6 = _terrainDesiredMoveSpeed;
    s.interactionCause = _interactionCauseSlot;
	s.interactionDepth = _interactionDepth;
    s.terrainTargetTileX = _terrainTargetTileX;
    s.terrainTargetTileY = _terrainTargetTileY;
    return true;
}

void LevelObjectDog::NetSet(const NetworkLevelObjectDynEntState& s) 
{
    _bounds.x = s.x; _bounds.y = s.y;
    _velocity.x = s.vx; _velocity.y = s.vy;
    if (s.mode >= static_cast<uint8_t>(DogState::Default) && s.mode <= static_cast<uint8_t>(DogState::Stunned)) _state = static_cast<DogState>(s.mode);
    else _state = DogState::Default;
    _isJumping = (s.flags & 1) != 0;
    _isSlidingLeft = (s.flags & 2) != 0;
    _isFacingLeft = (s.flags & 4) != 0;
    _inputJumpPrev = (s.flags & 8) != 0;
    _inputDigPrev = (s.flags & 16) != 0;
    _inputBarkPrev = (s.flags & 32) != 0;
    _isGrounded = (s.flags & 64) != 0;
    _canDoBarkJump = (s.flags & 128) != 0;
    if (s.aux >= static_cast<uint8_t>(SurfaceType::None) && s.aux < static_cast<uint8_t>(SurfaceType::Count)) _surfaceTypeStandingOn = static_cast<SurfaceType>(s.aux);
    else _surfaceTypeStandingOn = SurfaceType::None;
    _jumpTimer = s.t0; _coyoteTimer = s.t1; _jumpInputBufferTimer = s.t2;
    _inputAxisXPrev = s.t3;
    _stunTimer = std::min(s.t4, kStunMaxTime);
    _barkCooldownTimer = s.t5;
    if (s.interactionCause == kInteractionNoCause || (s.interactionCause >= 0 && s.interactionCause < kMaxSlots)) _interactionCauseSlot = s.interactionCause;
    else _interactionCauseSlot = kInteractionNoCause;
    _interactionDepth = s.interactionDepth;
    _terrainTargetTileX = std::min(s.terrainTargetTileX, static_cast<uint8_t>(kMaxLevelWidth - 1));
    _terrainTargetTileY = std::min(s.terrainTargetTileY, static_cast<uint8_t>(kMaxLevelHeight - 1));
    _terrainDesiredMoveSpeed = std::min(s.t6, kInputTerrainMoveSpeed);
}

void LevelObjectDog::CollidedWithX(LevelObject* other, float prevVelocityX)
{
    if (_state == DogState::Stunned) SetVelocityX(-prevVelocityX * kStunnedBounciness);
}

void LevelObjectDog::CollidedWithY(LevelObject* other, float prevVelocityY)
{
    if (_state == DogState::Stunned) 
    {
        // To prevent isGrounded flickering
        if (std::abs(prevVelocityY) < kStunnedBounceMinVelocityY) SetVelocityY(0.0f);
        else SetVelocityY(-prevVelocityY * kStunnedBounciness);
    }
}

void LevelObjectDog::OnDug(LevelObject* digger)
{
    if (digger == this) return;

    printf("Dog dug\n");
    Stun(digger, 1.5f);
}

void LevelObjectDog::OnForceField(const LevelObject* field)
{
    Stun(field, 1.5f);
}

void LevelObjectDog::OnTrampoline(const LevelObject* trampoline, float prevVelocityY)
{
    if (_state == DogState::InTerrain) return;

    _canDoBarkJump = true;
    LevelObject::OnTrampoline(trampoline, prevVelocityY);

    if (_state == DogState::Default && (_jumpInputBufferTimer > 0.0f || _inputJump))
    {
        SetVelocityY(GetVelocityY() * kTrampolineJumpBoost);
        _jumpInputBufferTimer = 0.0f;
        _coyoteTimer = 0.0f;
    }
}

void LevelObjectDog::OnEnteredTerrain(uint8_t tileX, uint8_t tileY, float velX, float velY, bool enteredByDigging)
{
    if (_state != DogState::Default && _state != DogState::Stunned) return;

    _terrainTargetTileX = tileX;
    _terrainTargetTileY = tileY;
    _state = DogState::InTerrain;

    if (enteredByDigging)
    {
        SetVelocityX(0.0f);
        SetVelocityY(0.0f);
        _terrainDesiredMoveSpeed = kInputTerrainMoveSpeed;
    }
    else
    {
        SetVelocityX(velX);
        SetVelocityY(velY);
        _terrainDesiredMoveSpeed = 0.0f;
    }
}