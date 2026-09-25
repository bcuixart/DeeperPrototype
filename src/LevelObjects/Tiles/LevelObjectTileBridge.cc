#include "LevelObjectTileBridge.hh"
#include "GameManager.hh"

LevelObjectTileBridge::LevelObjectTileBridge(const Vector2& position) : LevelObjectTile(position)
{
    _bounds = { position.x, position.y, 1.0f, 0.25f };
}

LevelObjectTileBridge::~LevelObjectTileBridge()
{

}

void LevelObjectTileBridge::Update(const float deltaTime)
{
}

void LevelObjectTileBridge::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::TilesetBridge);

    const float tile_size = 16;
    Rectangle src = { _spriteIndex * tile_size, 0, tile_size, tile_size };
    Rectangle dst = { _bounds.x, _bounds.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

LevelObjectTileType LevelObjectTileBridge::GetTileType() const
{
	return LevelObjectTileType::Bridge;
}

void LevelObjectTileBridge::OnDug(LevelObject* digger)
{
    if (LevelObjectDog* dog = dynamic_cast<LevelObjectDog*>(digger))
		dog->DropThroughSemisolid(GetBounds());
}