#include "LevelObjectBone.hh"

LevelObjectBone::LevelObjectBone(const Vector2& position)
{
    _bounds = { position.x + (1 - kWidth) / 2.0f, position.y + 0.25f, kWidth, kHeight };

    _isGrabbed = false;
}

void LevelObjectBone::Update(const float deltaTime) 
{

}

void LevelObjectBone::Render(const float deltaTime) const 
{
    if (_isGrabbed) return;

    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::Bone);

    Rectangle src = { 0.0f, 0.0f, 14.0f, 7.0f };
    Rectangle dst = {
        _bounds.x,
        _bounds.y,
        kWidth,
        kHeight
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectBone::OnOverlap(LevelObject* other)
{
    if (_isGrabbed) return;

    if (LevelObjectDog* dog = dynamic_cast<LevelObjectDog*>(other))
	{
        if (!dog->TryToGrabObject(this)) return;

        _isGrabbed = true;       

		return;
	}
}