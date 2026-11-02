#include "LevelBackgroundCity.hh"

LevelBackgroundCity::LevelBackgroundCity(const int width, const int height) :
	LevelBackground(width, height)
{
}

LevelBackgroundCity::~LevelBackgroundCity()
{
}

void LevelBackgroundCity::Update(const float deltaTime)
{
}

void LevelBackgroundCity::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::BackgroundCity);

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