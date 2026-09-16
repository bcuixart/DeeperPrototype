#include "LevelObjectTileSlope.hh"
#include "GameManager.hh"

LevelObjectTileSlope::LevelObjectTileSlope(const Vector2& position) : LevelObjectTile(position)
{

}

LevelObjectTileSlope::~LevelObjectTileSlope()
{

}

void LevelObjectTileSlope::Update(const float deltaTime)
{
}

void LevelObjectTileSlope::Render(const float deltaTime) const
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::TilesetGround);

    int col = _spriteIndex % 11;
    int row = _spriteIndex / 11;

    const float tile_size = 16;
    Rectangle src = { col * tile_size, row * tile_size, tile_size, tile_size };
    Rectangle dst = { _position.x, _position.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

LevelObjectTileType LevelObjectTileSlope::GetTileType() const
{
	return (_isSlopedTile ? LevelObjectTileType::Slope : LevelObjectTileType::Ground);
}

void LevelObjectTileSlope::SetSpriteIndex(uint8_t idx)
{
	_spriteIndex = idx;

    _isSlopedTile = (slopedTiles.find(idx) != slopedTiles.end());
    if (_isSlopedTile)
    {
        _spriteIndex = slopesSpriteMap.at(idx);
    }
}