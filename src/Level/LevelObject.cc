#include "LevelObject.hh"
#include "GameManager.hh"

LevelObject::LevelObject()
{
	_velocity = { 0.0f, 0.0f };
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

void LevelObject::RenderBounds(const float deltaTime, const Color& color) const
{
	DrawRectangleLinesEx(_bounds, 0.025f, color);
}

void LevelObject::SetSolidSideCollisionMask(const uint8_t side, const bool mask)
{
	if (mask) {
		_solidSidecollisionMask |= side;
	}
	else {
		_solidSidecollisionMask &= ~side;
	}
}