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
        case DogState::SlidingLeft:
            Update_Sliding(deltaTime, true);
            break;
        case DogState::SlidingRight:
            Update_Sliding(deltaTime, false);
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

    if (IsKeyDown(KEY_RIGHT)) desiredVelocityX = kWalkSpeed;
    else if (IsKeyDown(KEY_LEFT)) desiredVelocityX = -kWalkSpeed;

    if (desiredVelocityX != 0.0f)
    {
	    const bool belowOrOpposite = fabsf(currentVelocityX) < fabsf(desiredVelocityX)
	                                || (currentVelocityX >= 0.0f) != (desiredVelocityX >= 0.0f);
	    if (belowOrOpposite) SetVelocityX(desiredVelocityX);
    }
    else if (fabsf(currentVelocityX) <= kWalkSpeed)
    {
	    SetVelocityX(0.0f);
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

void LevelObjectDog::Update_Sliding(const float deltaTime, bool isSlidingLeft)
{
    if (GetVelocityY() <= 0.0f || GetVelocityX() == 0.0f)
    {
        _state = DogState::Default;
        return;
    }

    if (IsKeyDown(KEY_SPACE))
    {
        SetVelocityY(-12.0f);
        _state = DogState::Default;
        return;
    }

    // Move in a 45-degree angle downwards while sliding
    AddVelocityX(isSlidingLeft ? -kSlideAcceleration * deltaTime : kSlideAcceleration * deltaTime);
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

void LevelObjectDog::StartSlidingRight()
{
    std::cout << "StartSlidingRight called" << std::endl;
    if (_state != DogState::Default) return;
    if (!_isGrounded) return;

    _state = DogState::SlidingRight;
    if (GetVelocityX() == 0.0f) SetVelocityX(kSlideInitialSpeed);
    if (GetVelocityY() == 0.0f) SetVelocityY(kSlideInitialSpeed);
}

void LevelObjectDog::StartSlidingLeft()
{
    if (_state != DogState::Default) return;
    if (!_isGrounded) return;

    _state = DogState::SlidingLeft;
    if (GetVelocityX() == 0.0f) SetVelocityX(-kSlideInitialSpeed);
    if (GetVelocityY() == 0.0f) SetVelocityY(kSlideInitialSpeed);
}

void LevelObjectDog::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::DogTest);

    const float spriteSize = 2.0f;

    Rectangle src = { 0.0f, 0.0f, 32.0f, 32.0f };
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