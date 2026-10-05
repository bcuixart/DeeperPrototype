#ifndef MENUCURSOR_HH
#define MENUCURSOR_HH

#include <raylib.h>
#include <cstdint>
#include <string>

#include "InputManager.hh"

class MenuCursor {
public:
	MenuCursor(int playerIndex, bool isActive);
	~MenuCursor();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void ReceiveInput(uint8_t input);
	bool IsSelecting() const { return _selecting; }

	void SetActive(bool isActive) { _isActive = isActive; }
	bool IsActive() const { return _isActive; }

	float GetX() const { return _bounds.x; }
	float GetY() const { return _bounds.y; }

	int GetPlayerIndex() const { return _playerIndex; }

private:
	bool _isActive;
	int _playerIndex;

	Rectangle _bounds;

	uint8_t _input;
	uint8_t _prevInput;

	bool _selecting;

	static constexpr float kCursorSpeed = 200.0f;
};

#endif