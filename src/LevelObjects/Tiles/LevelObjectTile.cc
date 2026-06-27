#include "LevelObjectTile.hh"
#include "GameManager.hh"

LevelObjectTile::LevelObjectTile(const Vector2& position) : LevelObject(position)
{

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
