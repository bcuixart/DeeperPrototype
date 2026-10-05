#include "MenuObjectButton.hh"

MenuObjectButton::MenuObjectButton(const Rectangle& bounds, Callback onInteractCallback, const std::string& text) : _bounds(bounds), _onInteractCallback(std::move(onInteractCallback)), _text(text)
{
}

MenuObjectButton::~MenuObjectButton()
{
}

void MenuObjectButton::Update(const float deltaTime)
{
	// We set _isHovered to false here. It will be set to true by the MenuManager after Update.
	_isHovered = false;
}

void MenuObjectButton::Render(const float deltaTime)
{
	DrawRectangleLinesEx(_bounds, 1.0f, _isHovered ? GREEN : BLUE);
	DrawText(_text.c_str(), _bounds.x + 10, _bounds.y + 10, 20, WHITE);
}

bool MenuObjectButton::IsCursorHovering(const float x, const float y, const int cursorIndex) const
{
	return CheckCollisionPointRec(Vector2{x, y}, _bounds);
}

void MenuObjectButton::OnHover(const int cursorIndex)
{
	_isHovered = true;
}

void MenuObjectButton::OnInteract(const int cursorIndex)
{
	if (!_onInteractCallback) return;
	_onInteractCallback(cursorIndex);
}