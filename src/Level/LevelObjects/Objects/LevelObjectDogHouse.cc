#include "LevelObjectDogHouse.hh"
#include "Level/Level.hh"

LevelObjectDogHouse::LevelObjectDogHouse(const Vector2& position, Level* level)
    : _level(level)
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

    Rectangle src = { 0.0f, 0.0f, 20.0f, 25.0f };
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

void LevelObjectDogHouse::OnOverlap(LevelObject* other)
{
    if (LevelObjectDog* dog = dynamic_cast<LevelObjectDog*>(other))
	{
        if (dog->IsGone()) return;
        if (!dog->IsInDefaultState()) return;

        // If the dog is not holding a bone, it cannot place it in the dog house
        if (dog->GetHeldGrabbableType() != GrabbableType::Bone) return;

        uint8_t dogSlot = dog->GetSlot();
        if (dogSlot >= MAX_PLAYERS) return;

        // If the dog is not in a valid slot, it cannot place the bone
        if (!(_dogSlotsMask & (1u << dogSlot))) return;

        dog->LetGo();

        _level->ManagerNotifyScore(other, 10);
	}
}