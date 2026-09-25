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
            Update_SlidingLeft(deltaTime);
            break;
        case DogState::SlidingRight:
            Update_SlidingRight(deltaTime);
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

    if (IsKeyDown(KEY_RIGHT)) 
    {
        SetVelocityX(10.0f);
    } 
    else if (IsKeyDown(KEY_LEFT)) 
    {
        SetVelocityX(-10.0f);
    } 
    else 
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

void LevelObjectDog::Update_SlidingLeft(const float deltaTime)
{
}

void LevelObjectDog::Update_SlidingRight(const float deltaTime)
{
}

void LevelObjectDog::Update_InDirt(const float deltaTime)
{
}

void LevelObjectDog::Update_Flung(const float deltaTime)
{
}



void LevelObjectDog::DropThroughSemisolid(const Rectangle& semisolidBounds)
{
    constexpr float kDropThroughMargin = 0.05f;
	_bounds.y += kDropThroughMargin;
}

void LevelObjectDog::StartSlidingRight()
{
}

void LevelObjectDog::StartSlidingLeft()
{
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