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
    Rectangle dst = { _bounds.x, _bounds.y, 1.0f, 1.0f };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectTileSlope::RenderBounds(const float deltaTime, const Color& color) const
{
    switch(_slopeOrientation)
    {
        case SlopeOrientation::RisingRight:
            DrawLineEx({ _bounds.x, _bounds.y + 1.0f }, { _bounds.x + 1.0f, _bounds.y }, 0.025f, color);
            if (_spriteIndex == 47) DrawLineEx({ _bounds.x, _bounds.y + 1.0f }, { _bounds.x + 1.0f, _bounds.y + 1.0f }, 0.025f, color);
            break;
        case SlopeOrientation::RisingLeft:
            DrawLineEx({ _bounds.x, _bounds.y }, { _bounds.x + 1.0f, _bounds.y + 1.0f }, 0.025f, color);
            if (_spriteIndex == 48) DrawLineEx({ _bounds.x, _bounds.y + 1.0f }, { _bounds.x + 1.0f, _bounds.y + 1.0f }, 0.025f, color);
            break;
        case SlopeOrientation::FallingRight:
            DrawLineEx({ _bounds.x, _bounds.y }, { _bounds.x + 1.0f, _bounds.y + 1.0f }, 0.025f, color);
            break;
        case SlopeOrientation::FallingLeft:
            DrawLineEx({ _bounds.x, _bounds.y + 1.0f }, { _bounds.x + 1.0f, _bounds.y }, 0.025f, color);
            break;
        default:
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
            break;
    }
}

Vector2 LevelObjectTileSlope::SampleTopBottomAtX(const float x) const
{
    switch (_slopeOrientation)
    {
        case SlopeOrientation::RisingRight:
            return { _bounds.y + _bounds.height * (1.0f - (x - _bounds.x) / _bounds.width), _bounds.y + _bounds.height };
        case SlopeOrientation::RisingLeft:
            return { _bounds.y + _bounds.height * (x - _bounds.x) / _bounds.width, _bounds.y + _bounds.height };
        case SlopeOrientation::FallingRight:
            return { _bounds.y, _bounds.y + _bounds.height * (x - _bounds.x) / _bounds.width };
        case SlopeOrientation::FallingLeft:
            return { _bounds.y, _bounds.y + _bounds.height * (1.0f - (x - _bounds.x) / _bounds.width) };
        default:
            return { _bounds.y, _bounds.y + _bounds.height };
    }
}

Vector2 LevelObjectTileSlope::SampleLeftRightAtY(const float y) const
{
    switch (_slopeOrientation)
    {
        case SlopeOrientation::RisingRight:
            return { _bounds.x + _bounds.width * (1.0f - (y - _bounds.y) / _bounds.height), _bounds.x + _bounds.width };
        case SlopeOrientation::RisingLeft:
            return { _bounds.x, _bounds.x + _bounds.width * (y - _bounds.y) / _bounds.height };
        case SlopeOrientation::FallingRight:
            return { _bounds.x + _bounds.width * (y - _bounds.y) / _bounds.height, _bounds.x + _bounds.width };
        case SlopeOrientation::FallingLeft:
            return { _bounds.x, _bounds.x + _bounds.width * (1.0f - (y - _bounds.y) / _bounds.height) };
        default:
            return { _bounds.x, _bounds.x + _bounds.width };
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