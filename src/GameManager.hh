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

enum class PlayerControllerType {
	KEYBOARD_ARROW,
	KEYBOARD_WASD,
	CONTROLLER,
};

struct PlayerInfo
{
	bool isActive;
	char playerName[16];
	PlayerControllerType controllerType;
	int controllerIndex;
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

	void TryToAddPlayer(PlayerControllerType type);
	void RemovePlayer(uint8_t playerNum);

	GameState _gameState;
	PlayerInfo _playerInfo[LOCAL_PLAYERS];

	std::unique_ptr<AssetManager> _assetManager = nullptr;
	std::unique_ptr<LevelManager> _levelManager = nullptr;

	uint8_t _playersActive;

	bool _addedPlayerWASD{ false };
	bool _addedPlayerArrows{ false };
};

#endif