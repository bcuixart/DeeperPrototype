#ifndef INPUTMANAGER_HH
#define INPUTMANAGER_HH

#include <cstdint>
#include <cmath>
#include <array>
#include <raylib.h>

#define MAX_LOCAL_PLAYERS 4
#define MAX_GAMEPADS 4
#define MAX_PLAYERS 8

enum class PlayerControllerType {
	KEYBOARD_ARROW,
	KEYBOARD_WASD,
	KEYBOARD_IJKL,
	GAMEPAD_0,
	GAMEPAD_1,
	GAMEPAD_2,
	GAMEPAD_3,
};

struct PlayerInfo
{
	bool isActive;
	char playerName[16];
	PlayerControllerType controllerType;
	uint8_t gamepadIndex;
};

#define BITMASK_MENU_SELECT		0x01
#define BITMASK_MENU_SECONDARY	0x02
#define BITMASK_MENU_AXIS_X	    0x1C
#define BITMASK_MENU_AXIS_Y	    0xE0

#define BITMASK_LEVEL_JUMP		0x01
#define BITMASK_LEVEL_DIG		0x02
#define BITMASK_LEVEL_BARK		0x04
#define BITMASK_LEVEL_AXIS_X	0xF1

class InputManager {
public:
	InputManager();
	~InputManager();

	std::array<uint8_t, MAX_LOCAL_PLAYERS> GetMenuInput(const std::array<PlayerInfo, MAX_LOCAL_PLAYERS>& _playerInfo);
	std::array<uint8_t, MAX_LOCAL_PLAYERS> GetLevelInput(const std::array<PlayerInfo, MAX_LOCAL_PLAYERS>& _playerInfo);

	bool AnyKeyboardArrowKeyPressed() const;
	bool AnyKeyboardWASDKeyPressed() const;
	bool AnyKeyboardIJKLKeyPressed() const;
	bool AnyGamepadButtonPressed(int gamepadIndex) const;

protected:
	uint8_t GetMenuInput_KeyboardArrow();
	uint8_t GetMenuInput_KeyboardWASD();
	uint8_t GetMenuInput_KeyboardIJKL();
	uint8_t GetMenuInput_Gamepad(uint8_t gamepadIndex);

	uint8_t GetLevelInput_KeyboardArrow();
	uint8_t GetLevelInput_KeyboardWASD();
	uint8_t GetLevelInput_KeyboardIJKL();
	uint8_t GetLevelInput_Gamepad(uint8_t gamepadIndex);

	static constexpr float kStickDeadZone = 0.1f;
	static constexpr float kDigThreshold = 0.5f;
};

#endif