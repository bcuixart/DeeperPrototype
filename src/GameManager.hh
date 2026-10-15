#ifndef GAMEMANAGER_HH
#define GAMEMANAGER_HH

#include <iostream>
#include <vector> 
#include <array>
#include <algorithm> 
#include <memory> 
#include <string>

#include "AssetManager.hh"
#include "InputManager.hh"
#include "Network/NetworkManager.hh"
#include "Network/ClockSync.hh"
#include "Network/RemoteInputBuffer.hh"
#include "Level/LevelManager.hh"
#include "Level/LevelNetworkSync.hh"
#include "Menu/MenuManager.hh"
#include "Utils/DeterministicRNG.hh"

using namespace std;

constexpr float kFixedDeltaTime = 1.0f / 60.0f;

enum class GameState {
	MAIN_MENU,
	PLAYER_SETTINGS,
	LOBBY,
	LEVEL,
};

struct NetLaunchOptions
{
    enum class Mode { Offline, Host, Join };
    Mode mode = Mode::Offline;
    std::string address = "127.0.0.1";
    uint16_t port = 7777;
    int lagMs = 0;
};

class GameManager {
public:
	GameManager();
	~GameManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void PollNetwork();
	void PollInputToAddPlayers();

	bool TryToRemovePlayer(uint8_t playerNum);

	void OnPlayerSelectedStartGame() { _playerSelectedStartGame = true; };

	void SetPlayerDogType(uint8_t playerNum, uint8_t dogType) { if (playerNum < _playersActive) _playerInfo[playerNum].dogType = dogType; }

	float GetTimeScale() const;
	void SetNetLaunchOptions(const NetLaunchOptions& options) { _netOptions = options; }

	static GameManager* instance;

protected:

private:
	void SimulateTick(const std::array<uint8_t, MAX_PLAYERS>& slotInput, float deltaTime);

	void Update_PlayerSettings(const float deltaTime);

	bool TryToAddPlayer(PlayerControllerType type, uint8_t gamepadIndex);

	bool IsControllerTypeInUse(PlayerControllerType type) const;
	bool AnyGamepadButtonPressed(int gamepadIndex) const;
	void CheckForDisconnectedGamepadPlayers();

	void OnStartGamePressed();

	void EnterOnlineLobby();
	void LeaveOnline();

	void StartOfflineLevel(const std::string& levelName);
	void StartOnlineLevel(const LevelStartInfo& info);
	void BeginLevel(const std::string& levelName, const LevelPlayers& players, uint32_t seed);

	void RenderLobby();

	std::array<uint8_t, MAX_PLAYERS> BuildSlotInput();

	GameState _gameState;
	std::array<PlayerInfo, MAX_LOCAL_PLAYERS> _playerInfo;
	std::array<uint8_t, MAX_LOCAL_PLAYERS> _slotOfLocalPlayers;

	NetLaunchOptions _netOptions;
	uint32_t _levelTick = 0;
	RemoteInputBuffer _remoteInputs;
	std::vector<uint8_t> _mySlots;

	ClockSync _clockSync;
	uint32_t _inputLookups = 0;
	uint32_t _inputMisses = 0;

	static constexpr uint32_t kInputHistory = 64;
	std::array<std::array<uint8_t, MAX_PLAYERS>, kInputHistory> _usedInputs{};

	std::unique_ptr<AssetManager> _assetManager = nullptr;
	std::unique_ptr<InputManager> _inputManager = nullptr;
	std::unique_ptr<NetworkManager> _networkManager = nullptr;
	std::unique_ptr<MenuManager> _menuManager = nullptr;
	std::unique_ptr<LevelManager> _levelManager = nullptr;
	std::unique_ptr<LevelNetworkSync> _levelNetworkSync = nullptr;

	uint8_t _playersActive;

	bool _playerSelectedStartGame = false;
};

#endif