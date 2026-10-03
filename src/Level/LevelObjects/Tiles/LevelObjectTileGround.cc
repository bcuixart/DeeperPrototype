#include "LevelObjectTileGround.hh"
#include "GameManager.hh"

LevelObjectTileGround::LevelObjectTileGround(const Vector2& position) : LevelObjectTile(position)
{

}

LevelObjectTileGround::~LevelObjectTileGround()
{

}

void LevelObjectTileGround::Update(const float deltaTime)
{
}

void LevelObjectTileGround::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::TilesetGround);

    int col = _spriteIndex % 11;
    int row = _spriteIndex / 11;

    const float tile_size = 16;
    Rectangle src = { col * tile_size, row * tile_size, tile_size, tile_size };
    Rectangle dst = { _bounds.x, _bounds.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectTileGround::RenderBounds(const float deltaTime, const Color& color) const
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

LevelObjectTileType LevelObjectTileGround::GetTileType() const
{
	return LevelObjectTileType::Ground;
}
