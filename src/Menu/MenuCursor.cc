#include "MenuCursor.hh"

MenuCursor::MenuCursor(int playerIndex, bool isActive)
{
	_playerIndex = playerIndex;
	_isActive = isActive;

	_bounds = { 0, 0, 16, 16 };

	_input = 0;
	_prevInput = 0;

	_selecting = false;
}

MenuCursor::~MenuCursor()
{
}

void MenuCursor::Update(const float deltaTime)
{
	if (!_isActive) return;

	uint8_t axisX = (_input & BITMASK_MENU_AXIS_X) >> 2;
	uint8_t axisY = (_input & BITMASK_MENU_AXIS_Y) >> 5;

	float movementX = 0.0f;
	if (axisX == 0x00) movementX = -1.0f;
	else if (axisX == 0x01) movementX = -0.5f;
	else if (axisX == 0x02) movementX = -0.25f;
	else if (axisX == 0x05) movementX = 0.25f;
	else if (axisX == 0x06) movementX = 0.5f;
	else if (axisX == 0x07) movementX = 1.0f;

	float movementY = 0.0f;
	if (axisY == 0x00) movementY = -1.0f;
	else if (axisY == 0x01) movementY = -0.5f;
	else if (axisY == 0x02) movementY = -0.25f;
	else if (axisY == 0x05) movementY = 0.25f;
	else if (axisY == 0x06) movementY = 0.5f;
	else if (axisY == 0x07) movementY = 1.0f;

	_bounds.x += movementX * kCursorSpeed * deltaTime;
	_bounds.y += movementY * kCursorSpeed * deltaTime;

	_selecting = (_input & BITMASK_MENU_SELECT) != 0 && (_prevInput & BITMASK_MENU_SELECT) == 0;

	_prevInput = _input;
}

void MenuCursor::Render(const float deltaTime)
{
	if (!_isActive) return;

	DrawRectangleRec(_bounds, _isActive ? RED : GRAY);
	DrawText(std::to_string(_playerIndex + 1).c_str(), _bounds.x + 4, _bounds.y + 4, 8, WHITE);
}

void MenuCursor::ReceiveInput(uint8_t input)
{
	_input = input;
}