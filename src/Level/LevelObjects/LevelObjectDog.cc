#include "LevelObjectDog.hh"
#include "Level/LevelManager.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position, LevelManager* levelManager)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
    _levelManager = levelManager;
}

void LevelObjectDog::Update(const float deltaTime) 
{
    _coyoteTimer = _isGrounded ? kCoyoteTime : std::max(0.0f, _coyoteTimer - deltaTime);
    const bool jumpPressedThisFrame = _inputJump && !_inputJumpPrev;
    _jumpInputBufferTimer = jumpPressedThisFrame ? kJumpInputBufferTime : std::max(0.0f, _jumpInputBufferTimer - deltaTime);

    switch (_state)
    {
        case DogState::Default:
            Update_Default(deltaTime);
            break;
        case DogState::Sliding:
            Update_Sliding(deltaTime);
            break;
        case DogState::InDirt:
            Update_InDirt(deltaTime);
            break;
        case DogState::Flung:
            Update_Flung(deltaTime);
            break;
    }

    _inputJumpPrev = _inputJump;
    _inputDigPrev = _inputDig;
    _inputBarkPrev = _inputBark;
    _inputAxisXPrev = _inputAxisX;
}

void LevelObjectDog::Update_Default(const float deltaTime)
{
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

    float currentVelocityX = GetVelocityX();
    float desiredVelocityX = kWalkSpeed * _inputAxisX;

    if (_inputAxisX > 0.0f) _isFacingLeft = false;
    else if (_inputAxisX < 0.0f) _isFacingLeft = true;

    if (desiredVelocityX != 0.0f)
    {
        const bool belowOrOpposite = fabsf(currentVelocityX) < fabsf(desiredVelocityX)
                                    || (currentVelocityX >= 0.0f) != (desiredVelocityX >= 0.0f);
        if (belowOrOpposite) SetVelocityX(desiredVelocityX);
        else if (_isGrounded) SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }
    else if (fabsf(currentVelocityX) <= kWalkSpeed && (_inputAxisX == 0.0f && _inputAxisXPrev != 0.0f))
    {
        SetVelocityX(0.0f);
    }
    else if (_isGrounded)
    {
        SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }

    if (_inputDig && !_inputDigPrev) 
    {
        if (_isGrounded)
        {
            _levelManager->DigAt(this, { _bounds.x + kDigHorizontalOffset, _bounds.y + _bounds.height }, 
                                        { _bounds.x + _bounds.width - kDigHorizontalOffset, _bounds.y + _bounds.height });
        }
    }
}

void LevelObjectDog::Update_Sliding(const float deltaTime)
{
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

void LevelObjectDog::Update_InDirt(const float deltaTime)
{
    _coyoteTimer = 0.0f;
    _jumpInputBufferTimer = 0.0f;
    _isJumping = false;
    _jumpTimer = 0.0f;
}

void LevelObjectDog::Update_Flung(const float deltaTime)
{
    _coyoteTimer = 0.0f;
    _jumpInputBufferTimer = 0.0f;
    _isJumping = false;
    _jumpTimer = 0.0f;
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
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    

}

void LevelObjectDog::RenderBounds(const float deltaTime, const Color& color) const
{
    LevelObject::RenderBounds(deltaTime, color);

    DrawCircleV({ _bounds.x + kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
    DrawCircleV({ _bounds.x + _bounds.width - kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
}