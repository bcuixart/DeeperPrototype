#ifndef MENUCURSOR_HH
#define MENUCURSOR_HH

#include <raylib.h>
#include <cstdint>

#include "InputManager.hh"

class MenuCursor {
public:
	MenuCursor(int playerIndex, bool isActive);
	~MenuCursor();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void ReceiveInput(uint8_t input);

	void SetActive(bool isActive) { _isActive = isActive; }

private:
	bool _isActive;
	int _playerIndex;

	Rectangle _bounds;

	uint8_t _input;
	uint8_t _prevInput;

	static constexpr float kCursorSpeed = 200.0f;
};

#endif