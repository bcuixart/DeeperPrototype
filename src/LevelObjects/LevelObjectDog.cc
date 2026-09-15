#include "LevelObjectDog.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
}

void LevelObjectDog::Update(const float deltaTime) 
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
}
