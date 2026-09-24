#include "LevelObjectBallTennis.hh"

LevelObjectBallTennis::LevelObjectBallTennis(const Vector2& position)
{
    _bounds = { position.x + kWidth / 2.0f, position.y + kHeight / 2.0f, kWidth, kHeight };
    
    _velocity = { 10.0f, 0.0f };
}

void LevelObjectBallTennis::Update(const float deltaTime) 
{

}

void LevelObjectBallTennis::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::BallTennis);

    const float spriteSize = 1.0f;

    Rectangle src = { 0.0f, 0.0f, 16.0f, 16.0f };
    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}
