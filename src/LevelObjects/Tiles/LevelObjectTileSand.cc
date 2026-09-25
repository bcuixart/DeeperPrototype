#include "LevelObjectTileSand.hh"
#include "LevelManager.hh"

LevelObjectTileSand::LevelObjectTileSand(const Vector2& position, LevelManager* levelManager) : LevelObjectTile(position)
{
    _levelManager = levelManager;
}

LevelObjectTileSand::~LevelObjectTileSand()
{

}

void LevelObjectTileSand::Update(const float deltaTime)
{
}

void LevelObjectTileSand::Render(const float deltaTime) const
{
    if (_isBroken) return;

    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::TilesetSand);

    int col = _spriteIndex % 7;
    int row = _spriteIndex / 7;

    const float tile_size = 16;
    Rectangle src = { col * tile_size, row * tile_size, tile_size, tile_size };
    Rectangle dst = { _bounds.x, _bounds.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectTileSand::RenderBounds(const float deltaTime, const Color& color) const
{
    if (_isBroken) return;

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

LevelObjectTileType LevelObjectTileSand::GetTileType() const
{
	return LevelObjectTileType::Sand;
}

void LevelObjectTileSand::BreakableBreak(const LevelObject* other)
{
    if (_isBroken) return;

    _isBroken = true;

    _levelManager->UpdateAutotileNeighbors((int)GetPosition().x, (int)GetPosition().y);
}
