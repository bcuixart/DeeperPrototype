#include "MenuCursor.hh"

MenuCursor::MenuCursor(int playerIndex, bool isActive)
{
	_playerIndex = playerIndex;
	_isActive = isActive;

	_bounds = { 0, 0, 16, 16 };
}

MenuCursor::~MenuCursor()
{
}

void MenuCursor::Update(const float deltaTime)
{
}

void MenuCursor::Render(const float deltaTime)
{
	DrawRectangleRec(_bounds, _isActive ? RED : GRAY);
}