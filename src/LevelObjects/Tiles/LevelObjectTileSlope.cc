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

void LevelObjectTileSlope::RenderBounds(const float deltaTime, const Color& color) const
{
    switch(_slopeOrientation)
    {
        case SlopeOrientation::RisingRight:
            DrawTriangleLines({ _position.x + 1.0f, _position.y }, { _position.x, _position.y + 1.0f }, { _position.x + 1.0f, _position.y + 1.0f }, color);
            break;
        case SlopeOrientation::RisingLeft:
            DrawTriangleLines({ _position.x, _position.y }, { _position.x, _position.y + 1.0f }, { _position.x + 1.0f, _position.y + 1.0f }, color);
            break;
        case SlopeOrientation::FallingRight:
            DrawTriangleLines({ _position.x + 1.0f, _position.y + 1.0f }, { _position.x, _position.y }, { _position.x + 1.0f, _position.y }, color);
            break;
        case SlopeOrientation::FallingLeft:
            DrawTriangleLines({ _position.x, _position.y + 1.0f }, { _position.x, _position.y }, { _position.x + 1.0f, _position.y }, color);
            break;
        default:
            DrawRectangleLinesEx(_bounds, 0.025f, color);
            break;
    }
}

LevelObjectTileType LevelObjectTileSlope::GetTileType() const
{
	return LevelObjectTileType::Slope;
}

void LevelObjectTileSlope::SetSpriteIndex(uint8_t idx)
{
	_spriteIndex = idx;

    _isSlopedTile = (slopedSpriteIndexes.find(idx) != slopedSpriteIndexes.end());
    if (!_isSlopedTile)
    {
        _slopeOrientation = SlopeOrientation::None;
    }
    else
    {
        auto it = slopeSpriteIndexToOrientation.find(idx);
        if (it != slopeSpriteIndexToOrientation.end())
        {
            _slopeOrientation = it->second;
        }
        else
        {
            _slopeOrientation = SlopeOrientation::None;
        }
    }
}