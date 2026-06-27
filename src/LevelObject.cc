#include "LevelObject.hh"
#include "GameManager.hh"

LevelObject::LevelObject(const Vector2& position)
{
	_position = position;
}

LevelObject::~LevelObject()
{
}

void LevelObject::Update(const float deltaTime)
{
}

void LevelObject::Render(const float deltaTime) const
{
}

Vector2 LevelObject::Position() const
{
	return _position;
}