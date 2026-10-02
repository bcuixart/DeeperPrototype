#include "LevelObjectDog.hh"
#include "LevelManager.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position, LevelManager* levelManager)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
    _levelManager = levelManager;
}

void LevelObjectDog::Update(const float deltaTime) 
{
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
}

void LevelObjectDog::Update_Default(const float deltaTime)
{
    if (IsKeyDown(KEY_SPACE)) 
    {
        if (_isGrounded) SetVelocityY(-12.0f);
    } 

    float currentVelocityX = GetVelocityX();
    float desiredVelocityX = 0.0f;

    if (IsKeyDown(KEY_RIGHT)) { desiredVelocityX = kWalkSpeed; _isFacingLeft = false; }
    else if (IsKeyDown(KEY_LEFT)) { desiredVelocityX = -kWalkSpeed; _isFacingLeft = true; }

    if (desiredVelocityX != 0.0f)
    {
        const bool belowOrOpposite = fabsf(currentVelocityX) < fabsf(desiredVelocityX)
                                    || (currentVelocityX >= 0.0f) != (desiredVelocityX >= 0.0f);
        if (belowOrOpposite) SetVelocityX(desiredVelocityX);
        else if (_isGrounded) SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }
    else if (fabsf(currentVelocityX) <= kWalkSpeed && (IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_RIGHT)))
    {
        SetVelocityX(0.0f);
    }
    else if (_isGrounded)
    {
        SetVelocityX(currentVelocityX * powf(kGroundFriction, deltaTime));
    }

    if (IsKeyPressed(KEY_DOWN)) 
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

    if (IsKeyDown(KEY_SPACE))
    {
        SetVelocityY(-12.0f);
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
}

void LevelObjectDog::Update_Flung(const float deltaTime)
{
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
    if (GetVelocityX() == 0.0f) SetVelocityX(startingOnLeftSlope ? -kSlideInitialSpeed : kSlideInitialSpeed);
    if (GetVelocityY() == 0.0f) SetVelocityY(kSlideInitialSpeed);
}

void LevelObjectDog::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::DogTest);

    const float spriteSize = 2.0f;

    Rectangle src = { 0.0f, 0.0f, _isFacingLeft ? -32.0f : 32.0f, 32.0f };
    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    
    DrawCircleV({ _bounds.x + kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
    DrawCircleV({ _bounds.x + _bounds.width - kDigHorizontalOffset, _bounds.y + _bounds.height }, 0.1f, RED);
}