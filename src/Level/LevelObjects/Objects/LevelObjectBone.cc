#include "LevelObjectBone.hh"

LevelObjectBone::LevelObjectBone(const Vector2& position)
{
    _bounds = { position.x + (1 - kWidth) / 2.0f, position.y + 0.25f, kWidth, kHeight };
}

void LevelObjectBone::Update(const float deltaTime) 
{

}

void LevelObjectBone::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::Bone);

    Rectangle src = { 0.0f, 0.0f, 14.0f, 7.0f };
	if (fabsf(GetVelocityX()) >= GetInteractionVelocityX() || fabsf(GetVelocityY()) >= GetInteractionVelocityY()) src = {0.0f, 16.0f, 16.0f, 16.0f}; 
    Rectangle dst = {
        _bounds.x,
        _bounds.y,
        kWidth,
        kHeight
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}
