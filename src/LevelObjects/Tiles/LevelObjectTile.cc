#include "LevelObjectTile.hh"
#include "GameManager.hh"

LevelObjectTile::LevelObjectTile(const Vector2& position)
{
	_bounds = { position.x, position.y, 1.0f, 1.0f };
	_position = position;
}

LevelObjectTile::~LevelObjectTile()
{

}

void LevelObjectTile::Update(const float deltaTime)
{
}

void LevelObjectTile::Render(const float deltaTime) const
{
}

LevelObjectTileType LevelObjectTile::GetTileType() const
{
	return LevelObjectTileType::None;
}

void LevelObjectTile::SetSpriteIndex(uint8_t idx)
{
	_spriteIndex = idx;
}

uint8_t LevelObjectTile::GetSpriteIndex() const
{
	return _spriteIndex;
}