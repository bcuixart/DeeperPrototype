#ifndef GAMEMANAGER_HH
#define GAMEMANAGER_HH

#include <iostream>
#include <vector> 
#include <algorithm> 
#include <memory> 

#include <raylib.h>
#include <raymath.h>

#include "AssetManager.hh"
#include "LevelManager.hh"

using namespace std;

#define LOCAL_PLAYERS 4
#define MAX_GAMEPADS 4

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

enum class GameState {
	MAIN_MENU,
	PLAYER_SETTINGS,
	PLAYING,
};

class GameManager {
public:
	GameManager();
	~GameManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	static GameManager* instance;

protected:

private:
	void Update_PlayerSettings(const float deltaTime);
	void Render_PlayerSettings(const float deltaTime);

	bool TryToAddPlayer(PlayerControllerType type, uint8_t gamepadIndex);
	bool TryToRemovePlayer(uint8_t playerNum);

	bool IsControllerTypeInUse(PlayerControllerType type) const;
	bool AnyGamepadButtonPressed(int gamepadIndex) const;
	void CheckForDisconnectedGamepadPlayers();

	GameState _gameState;
	PlayerInfo _playerInfo[LOCAL_PLAYERS];

	std::unique_ptr<AssetManager> _assetManager = nullptr;
	std::unique_ptr<LevelManager> _levelManager = nullptr;

	uint8_t _playersActive;
};

#endif