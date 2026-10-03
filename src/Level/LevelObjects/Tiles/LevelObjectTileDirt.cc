#include "LevelObjectTileDirt.hh"
#include "GameManager.hh"

LevelObjectTileDirt::LevelObjectTileDirt(const Vector2& position) : LevelObjectTile(position)
{

}

LevelObjectTileDirt::~LevelObjectTileDirt()
{

}

void LevelObjectTileDirt::Update(const float deltaTime)
{
}

void LevelObjectTileDirt::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::TilesetDirt);

    int col = _spriteIndex % 7;
    int row = _spriteIndex / 7;

    const float tile_size = 16;
    Rectangle src = { col * tile_size, row * tile_size, tile_size, tile_size };
    Rectangle dst = { _bounds.x, _bounds.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectTileDirt::RenderBounds(const float deltaTime, const Color& color) const
{
    if (GetSolidSideCollisionMask(SOLID_SIDE_TOP)) {
        DrawLineEx({ _bounds.x, _bounds.y }, { _bounds.x + _bounds.width, _bounds.y }, 0.025f, color);
    }
    if (GetSolidSideCollisionMask(SOLID_SIDE_RIGHT)) {
        DrawLineEx({ _bounds.x + _bounds.width, _bounds.y }, { _bounds.x + _bounds.width, _bounds.y + _bounds.height }, 0.025f, color);
    }
    if (GetSolidSideCollisionMask(SOLID_SIDE_BOTTOM)) {
        DrawLineEx({ _bounds.x + _bounds.width, _bounds.y + _bounds.height }, { _bounds.x, _bounds.y + _bounds.height }, 0.025f, color);
    }
    if (GetSolidSideCollisionMask(SOLID_SIDE_LEFT)) {
        DrawLineEx({ _bounds.x, _bounds.y + _bounds.height }, { _bounds.x, _bounds.y }, 0.025f, color);
    }
}

LevelObjectTileType LevelObjectTileDirt::GetTileType() const
{
	return LevelObjectTileType::Dirt;
}
