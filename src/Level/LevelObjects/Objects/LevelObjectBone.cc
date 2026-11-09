#include "LevelObjectBone.hh"

LevelObjectBone::LevelObjectBone(const Vector2& position)
{
    _bounds = { position.x + (1 - kWidth) / 2.0f, position.y + 0.25f, kWidth, kHeight };

    _wasGrabbed = false;
}

void LevelObjectBone::Update(const float deltaTime) 
{

}

void LevelObjectBone::Render(const float deltaTime) const 
{
    if (_wasGrabbed) return;

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
    if (_wasGrabbed) return;

    if (LevelObjectDog* dog = dynamic_cast<LevelObjectDog*>(other))
	{
        if (!dog->TryToGrabObject(this)) return;

        _wasGrabbed = true;       

		return;
	}
}

bool LevelObjectBone::NetGet(NetworkLevelObjectDynEntState& s) const 
{
	s.flags = _wasGrabbed ? 1 : 0;
	return true;
}

void LevelObjectBone::NetSet(const NetworkLevelObjectDynEntState& s) 
{
    _wasGrabbed = (s.flags & 1) != 0;
}