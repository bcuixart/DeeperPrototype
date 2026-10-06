#ifndef GAMEMANAGER_HH
#define GAMEMANAGER_HH

#include <iostream>
#include <vector> 
#include <array>
#include <algorithm> 
#include <memory> 

#include "AssetManager.hh"
#include "InputManager.hh"
#include "Level/LevelManager.hh"
#include "Menu/MenuManager.hh"

using namespace std;

enum class GameState {
	MAIN_MENU,
	PLAYER_SETTINGS,
	LEVEL,
};

class GameManager {
public:
	GameManager();
	~GameManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void PollInputToAddPlayers();

	bool TryToRemovePlayer(uint8_t playerNum);

	void OnPlayerSelectedStartGame() { _playerSelectedStartGame = true; };

	void SetPlayerDogType(uint8_t playerNum, uint8_t dogType) { if (playerNum < _playersActive) _playerInfo[playerNum].dogType = dogType; }

	static GameManager* instance;

protected:

private:
	void Update_PlayerSettings(const float deltaTime);

	void StartLevel(const std::string& levelName);

	bool TryToAddPlayer(PlayerControllerType type, uint8_t gamepadIndex);

	bool IsControllerTypeInUse(PlayerControllerType type) const;
	bool AnyGamepadButtonPressed(int gamepadIndex) const;
	void CheckForDisconnectedGamepadPlayers();

	GameState _gameState;
	std::array<PlayerInfo, MAX_LOCAL_PLAYERS> _playerInfo;

	std::unique_ptr<AssetManager> _assetManager = nullptr;
	std::unique_ptr<InputManager> _inputManager = nullptr;
	std::unique_ptr<MenuManager> _menuManager = nullptr;
	std::unique_ptr<LevelManager> _levelManager = nullptr;

	uint8_t _playersActive;

	bool _playerSelectedStartGame = false;
};

#endif