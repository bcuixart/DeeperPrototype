#include "LevelObjectDog.hh"

LevelObjectDog::LevelObjectDog(const Vector2& position)
{
    _bounds = { position.x, position.y, kWidth, kHeight };
}

void LevelObjectDog::Update(const float deltaTime) 
{

}

void LevelObjectDog::Render(const float deltaTime) const {
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
