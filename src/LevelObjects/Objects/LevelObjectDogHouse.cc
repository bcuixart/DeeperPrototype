#include "LevelObjectDogHouse.hh"

LevelObjectDogHouse::LevelObjectDogHouse(const Vector2& position)
{
    _bounds = { position.x + (1.0f - kWidth) / 2.0f, position.y + (1.0f - kHeight), kWidth, kHeight };
}

void LevelObjectDogHouse::Update(const float deltaTime) 
{

}

void LevelObjectDogHouse::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::DogHouse);

    constexpr float kPixelsPerTile = 16.0f;

    Rectangle src = { 0.0f, 0.0f, 22.0f, 26.0f };
    const float spriteWidth  = src.width  / kPixelsPerTile;
    const float spriteHeight = src.height / kPixelsPerTile;

    const float groundY = _bounds.y + _bounds.height;

    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteWidth / 2.0f,
        groundY + (1.0f / kPixelsPerTile) - spriteHeight,
        spriteWidth,
        spriteHeight
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}
