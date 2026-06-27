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
	DrawRectangle( _position.x, _position.y, 1, 1, { 255, 127, 0, 255 });
}

LevelObjectTileType LevelObjectTileGround::GetTileType() const
{
	return LevelObjectTileType::Ground;
}
