#include "InputManager.hh"

InputManager::InputManager()
{
}

InputManager::~InputManager()
{
}

std::array<uint8_t, MAX_LOCAL_PLAYERS> InputManager::GetMenuInput(const std::array<PlayerInfo, MAX_LOCAL_PLAYERS>& _playerInfo)
{
	std::array<uint8_t, MAX_LOCAL_PLAYERS> input = { 0 };

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (!_playerInfo[i].isActive) continue;
		switch (_playerInfo[i].controllerType)
		{
		case PlayerControllerType::KEYBOARD_ARROW:
			input[i] = GetMenuInput_KeyboardArrow();
			break;
		case PlayerControllerType::KEYBOARD_WASD:
			input[i] = GetMenuInput_KeyboardWASD();
			break;
		case PlayerControllerType::KEYBOARD_IJKL:
			input[i] = GetMenuInput_KeyboardIJKL();
			break;
		default: // Gamepads
			input[i] = GetMenuInput_Gamepad(_playerInfo[i].gamepadIndex);
			break;
		}
	}

	return input;
}

uint8_t InputManager::GetMenuInput_KeyboardArrow()
{
	uint8_t select = IsKeyDown(KEY_ENTER) ? 1 : 0;
	uint8_t secondary = IsKeyDown(KEY_RIGHT_CONTROL) ? 1 : 0;

	uint8_t up = IsKeyDown(KEY_UP) ? 1 : 0;
	uint8_t down = IsKeyDown(KEY_DOWN) ? 1 : 0;
	uint8_t left = IsKeyDown(KEY_LEFT) ? 1 : 0;
	uint8_t right = IsKeyDown(KEY_RIGHT) ? 1 : 0;

	uint8_t axisX = (right) ? 0x07 : (left) ? 0x00 : 0x04;
	uint8_t axisY = (down) ? 0x07 : (up) ? 0x00 : 0x04;

	return (axisY << 5) | (axisX << 2) | (secondary << 1) | (select << 0);
}

uint8_t InputManager::GetMenuInput_KeyboardWASD()
{
	uint8_t select = IsKeyDown(KEY_LEFT_CONTROL) ? 1 : 0;
	uint8_t secondary = IsKeyDown(KEY_LEFT_SHIFT) ? 1 : 0;

	uint8_t up = IsKeyDown(KEY_W) ? 1 : 0;
	uint8_t down = IsKeyDown(KEY_S) ? 1 : 0;
	uint8_t left = IsKeyDown(KEY_A) ? 1 : 0;
	uint8_t right = IsKeyDown(KEY_D) ? 1 : 0;

	uint8_t axisX = (right) ? 0x07 : (left) ? 0x00 : 0x04;
	uint8_t axisY = (down) ? 0x07 : (up) ? 0x00 : 0x04;

	return (axisY << 5) | (axisX << 2) | (secondary << 1) | (select << 0);
}

uint8_t InputManager::GetMenuInput_KeyboardIJKL()
{
	uint8_t select = IsKeyDown(KEY_U) ? 1 : 0;
	uint8_t secondary = IsKeyDown(KEY_O) ? 1 : 0;

	uint8_t up = IsKeyDown(KEY_I) ? 1 : 0;
	uint8_t down = IsKeyDown(KEY_K) ? 1 : 0;
	uint8_t left = IsKeyDown(KEY_J) ? 1 : 0;
	uint8_t right = IsKeyDown(KEY_L) ? 1 : 0;

	uint8_t axisX = (right) ? 0x07 : (left) ? 0x00 : 0x04;
	uint8_t axisY = (down) ? 0x07 : (up) ? 0x00 : 0x04;

	return (axisY << 5) | (axisX << 2) | (secondary << 1) | (select << 0);
}

uint8_t InputManager::GetMenuInput_Gamepad(uint8_t gamepadIndex)
{
	constexpr float kStickDeadZone = 0.1f;

	uint8_t select = (IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_UP) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) ? 1 : 0;

	uint8_t secondary = (IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ||
						IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) ? 1 : 0;

	uint8_t up_dpad = IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_FACE_UP) ? 1 : 0;
	uint8_t down_dpad = IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_FACE_DOWN) ? 1 : 0;
	uint8_t left_dpad = IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_FACE_LEFT) ? 1 : 0;
	uint8_t right_dpad = IsGamepadButtonDown(gamepadIndex, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) ? 1 : 0;

	// Prioritize left joystick, use right joystick if the left one is not used
	float joystickX = GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_LEFT_X);
	float joystickY = GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_LEFT_Y);
	if (fabsf(joystickX) < kStickDeadZone) joystickX = GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_RIGHT_X);
	if (fabsf(joystickY) < kStickDeadZone) joystickY = GetGamepadAxisMovement(gamepadIndex, GAMEPAD_AXIS_RIGHT_Y);

	auto EncodeStickAxis = [kStickDeadZone](float value) -> uint8_t
	{
		if (fabsf(value) < kStickDeadZone) return 0x04;
		if (value > 0.0f) return (value > 0.75f) ? 0x07 : (value > 0.5f) ? 0x06 : 0x05;
		return (value < -0.75f) ? 0x00 : (value < -0.5f) ? 0x01 : 0x02;
	};

	// DPAD takes priority over joystick input
	uint8_t axisX = (right_dpad) ? 0x07 : (left_dpad) ? 0x00 : EncodeStickAxis(joystickX);
	uint8_t axisY = (down_dpad) ? 0x07 : (up_dpad) ? 0x00 : EncodeStickAxis(joystickY);

	return (axisY << 5) | (axisX << 2) | (secondary << 1) | (select << 0);
}

std::array<uint8_t, MAX_LOCAL_PLAYERS> InputManager::GetLevelInput(const std::array<PlayerInfo, MAX_LOCAL_PLAYERS>& _playerInfo)
{
	std::array<uint8_t, MAX_LOCAL_PLAYERS> input = { 0 };

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		if (!_playerInfo[i].isActive) continue;
		switch (_playerInfo[i].controllerType)
		{
		case PlayerControllerType::KEYBOARD_ARROW:
			input[i] = GetLevelInput_KeyboardArrow();
			break;
		case PlayerControllerType::KEYBOARD_WASD:
			input[i] = GetLevelInput_KeyboardWASD();
			break;
		case PlayerControllerType::KEYBOARD_IJKL:
			input[i] = GetLevelInput_KeyboardIJKL();
			break;
		default: // Gamepads
			input[i] = GetLevelInput_Gamepad(_playerInfo[i].gamepadIndex);
			break;
		}
	}

	return input;
}

uint8_t InputManager::GetLevelInput_KeyboardArrow()
{
	return 0;
}

uint8_t InputManager::GetLevelInput_KeyboardWASD()
{
	return 0;
}

uint8_t InputManager::GetLevelInput_KeyboardIJKL()
{
	return 0;
}

uint8_t InputManager::GetLevelInput_Gamepad(uint8_t gamepadIndex)
{
	return 0;
}