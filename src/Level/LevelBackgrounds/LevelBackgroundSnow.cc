#include "LevelBackgroundSnow.hh"

LevelBackgroundSnow::LevelBackgroundSnow(const int width, const int height) :
	LevelBackground(width, height)
{
}

LevelBackgroundSnow::~LevelBackgroundSnow()
{
}

void LevelBackgroundSnow::Update(const float deltaTime)
{
}

void LevelBackgroundSnow::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::BackgroundSnow);

    constexpr float kPixelsPerTile = 16.0f;

    Rectangle src = { 0.0f, 0.0f, _width * kPixelsPerTile, _height * kPixelsPerTile };
    Rectangle dst = {
        0,
        0,
        (float)_width,
        (float)_height
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}