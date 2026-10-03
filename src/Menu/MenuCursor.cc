#include "MenuCursor.hh"

MenuCursor::MenuCursor(int playerIndex, bool isActive)
{
	_playerIndex = playerIndex;
	_isActive = isActive;
}

MenuCursor::~MenuCursor()
{
}

void MenuCursor::Update(const float deltaTime)
{
}

void MenuCursor::Render(const float deltaTime)
{
}