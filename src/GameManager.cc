#include "GameManager.hh"
#include "Network/Transport/EnetTransport.hh"
#include "Network/Transport/LagSimTransport.hh"
#include <random>

static_assert(MAX_PLAYERS == kMaxSlots, "MAX_PLAYERS ha de coincidir amb kMaxSlots de la xarxa");

GameManager* GameManager::instance = nullptr;

GameManager::GameManager()
{
    GameManager::instance = this;
	_gameState = GameState::PLAYER_SETTINGS;

	_assetManager = std::make_unique<AssetManager>();
	_inputManager = std::make_unique<InputManager>();

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		_playerInfo[i].isActive = false;
		_slotOfLocalPlayers[i] = i;
	}
	_playersActive = 0;

	_menuManager = std::make_unique<MenuManager>();
}

GameManager::~GameManager()
{
    GameManager::instance = nullptr;
}

void GameManager::Update(const float deltaTime)
{
	switch (_gameState)
	{
	case GameState::MAIN_MENU:
		break;
	case GameState::PLAYER_SETTINGS:
		Update_PlayerSettings(deltaTime);
		_menuManager->ReceiveInput(_inputManager->GetMenuInput(_playerInfo));
		_menuManager->Update(deltaTime);
		break;
	case GameState::LOBBY:
		break;
	case GameState::LEVEL:
		_levelManager->ReceiveInput(BuildSlotInput());
		_levelManager->Update(deltaTime);
		break;
	}
}

void GameManager::Render(const float deltaTime) 
{
	switch (_gameState)
	{
	case GameState::MAIN_MENU:
		break;
	case GameState::PLAYER_SETTINGS:
		_menuManager->Render(deltaTime);
		break;
	case GameState::LOBBY:
		RenderLobby();
		break;
	case GameState::LEVEL:
		_levelManager->Render(deltaTime);
		break;
	}
}

void GameManager::Update_PlayerSettings(const float deltaTime)
{
	if (_playerSelectedStartGame) 
	{
		_playerSelectedStartGame = false;
		OnStartGamePressed();
	}
}

void GameManager::PollInputToAddPlayers()
{
	if (_gameState != GameState::PLAYER_SETTINGS) return;

	if (_inputManager->AnyKeyboardArrowKeyPressed()) TryToAddPlayer(PlayerControllerType::KEYBOARD_ARROW, 0);
	if (_inputManager->AnyKeyboardWASDKeyPressed()) TryToAddPlayer(PlayerControllerType::KEYBOARD_WASD, 0);
	if (_inputManager->AnyKeyboardIJKLKeyPressed()) TryToAddPlayer(PlayerControllerType::KEYBOARD_IJKL, 0);

	for (int gamepadIndex = 0; gamepadIndex < MAX_GAMEPADS; gamepadIndex++)
	{
		if (!IsGamepadAvailable(gamepadIndex)) continue;
		if (_inputManager->AnyGamepadButtonPressed(gamepadIndex))
		{
			TryToAddPlayer((PlayerControllerType)((int)PlayerControllerType::GAMEPAD_0 + gamepadIndex), gamepadIndex);
		}
	}

	CheckForDisconnectedGamepadPlayers();
}

bool GameManager::TryToAddPlayer(PlayerControllerType type, uint8_t gamepadIndex)
{
	if (_playersActive >= MAX_LOCAL_PLAYERS) return false;
	if (IsControllerTypeInUse(type)) return false;

	_menuManager->SetMenuCursorActive(_playersActive, true);
	_playerInfo[_playersActive].isActive = true;
	_playerInfo[_playersActive].controllerType = type;
	_playerInfo[_playersActive].gamepadIndex = gamepadIndex;
	_playerInfo[_playersActive].dogType = 0;
	_playersActive++;


	printf("Added player %d with controller type %d\n", _playersActive, (int)type);

	return true;
}

bool GameManager::TryToRemovePlayer(uint8_t playerNum)
{
    if (playerNum >= _playersActive) return false;
    if (!_playerInfo[playerNum].isActive) return false;

    const uint8_t activeBefore = _playersActive;

    for (int i = playerNum; i < _playersActive - 1; i++)
        _playerInfo[i] = _playerInfo[i + 1];
    _playerInfo[_playersActive - 1].isActive = false;
    _playersActive--;

    _menuManager->RemoveCursorAndShift(playerNum, activeBefore);
    return true;
}

bool GameManager::IsControllerTypeInUse(PlayerControllerType type) const
{
	for (int i = 0; i < _playersActive; i++)
	{
		if (_playerInfo[i].isActive && _playerInfo[i].controllerType == type) return true;
	}

	return false;
}

void GameManager::CheckForDisconnectedGamepadPlayers()
{
	for (int i = 0; i < _playersActive; i++)
	{
		PlayerControllerType type = _playerInfo[i].controllerType;
		if (type < PlayerControllerType::GAMEPAD_0) continue;

		int gamepadIndex = (int)type - (int)PlayerControllerType::GAMEPAD_0;
		if (!IsGamepadAvailable(gamepadIndex))
		{
			printf("Player %d's gamepad disconnected, removing\n", i);
			TryToRemovePlayer(i);
			i--;
		}
	}
}

void GameManager::OnStartGamePressed()
{
    if (_playersActive == 0) return;

    if (_netOptions.mode == NetLaunchOptions::Mode::Offline) StartOfflineLevel("test_level");
    else EnterOnlineLobby();
}

void GameManager::StartOfflineLevel(const std::string& levelName)
{
    LevelPlayers players;
    for (int i = 0; i < MAX_LOCAL_PLAYERS; i++) _slotOfLocalPlayers[i] = i;
    for (int i = 0; i < _playersActive; i++)
    {
        players[_slotOfLocalPlayers[i]].active = true;
        players[_slotOfLocalPlayers[i]].dogType = _playerInfo[i].dogType;
    }

    static std::mt19937 rng{ std::random_device{}() };
    BeginLevel(levelName, players, rng());
}

void GameManager::StartOnlineLevel(const LevelStartInfo& info)
{
    const std::vector<uint8_t> mine = _networkManager->GetMySlots();
    if ((int)mine.size() != _playersActive)
        printf("[net] AVIS: tinc %d jugadors locals i %d slots\n", _playersActive, (int)mine.size());

    for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
    {
        if (i >= _playersActive || i >= (int)mine.size()) break;
        _slotOfLocalPlayers[i] = mine[i];
    }

    LevelPlayers players;
    for (int slot = 0; slot < MAX_PLAYERS; slot++)
    {
        players[slot].active = info.lobby.slots[slot].Occupied();
        players[slot].dogType = info.lobby.slots[slot].dogType;
    }

    BeginLevel(info.levelName, players, info.spawnSeed);
}

void GameManager::BeginLevel(const std::string& levelName, const LevelPlayers& players, uint32_t seed)
{
    _gameState = GameState::LEVEL;
    _levelManager = std::make_unique<LevelManager>();
    _levelManager->LoadLevel(levelName, players, seed);
}

void GameManager::EnterOnlineLobby()
{
    std::vector<uint8_t> dogTypes;
    for (int i = 0; i < _playersActive; i++) dogTypes.push_back(_playerInfo[i].dogType);

    std::unique_ptr<ITransport> transport = std::make_unique<EnetTransport>();
    if (_netOptions.lagMs > 0)
    {
        transport = std::make_unique<LagSimTransport>(std::move(transport),
            LagSimSettings{ _netOptions.lagMs, _netOptions.lagMs / 4, 0.0f });
    }
    _networkManager = std::make_unique<NetworkManager>(std::move(transport));

    if (_netOptions.mode == NetLaunchOptions::Mode::Host)
    {
        if (!_networkManager->Host(_netOptions.port, dogTypes))
        {
            printf("[net] No s'ha pogut obrir la sala\n");
            LeaveOnline();
            return;
        }
    }
    else
    {
        _networkManager->Join(_netOptions.address, _netOptions.port, dogTypes);
    }

    _gameState = GameState::LOBBY;
}

void GameManager::LeaveOnline()
{
    if (_networkManager)
    {
        _networkManager->Leave();
        _networkManager.reset();
    }
}

void GameManager::PollNetwork()
{
    if (!_networkManager) return;

    _networkManager->Update();

    if (_networkManager->GetState() == NetworkManager::State::Failed)
    {
        printf("[net] %s\n", _networkManager->GetFailReason().c_str());
        LeaveOnline();
        _levelManager.reset();
        _gameState = GameState::PLAYER_SETTINGS;
        return;
    }

    if (_gameState == GameState::LOBBY && _networkManager->IsHost() && IsKeyPressed(KEY_ENTER))
    {
        _networkManager->HostStartLevel("test_level");
    }

    LevelStartInfo info;
    if (_networkManager->PopLevelStart(info)) StartOnlineLevel(info);
}

void GameManager::RenderLobby()
{
    BeginDrawing();
    ClearBackground({ 30, 30, 40, 255 });

    if (_networkManager && _networkManager->GetState() == NetworkManager::State::Connecting)
    {
        DrawText("Connectant...", 40, 40, 30, WHITE);
    }
    else if (_networkManager)
    {
        DrawText(_networkManager->IsHost() ? "Sala (host)" : "Sala", 40, 40, 30, WHITE);

        const LobbyState& lobby = _networkManager->GetLobby();
        for (int s = 0; s < MAX_PLAYERS; s++)
        {
            const SlotInfo& slot = lobby.slots[s];
            if (slot.Occupied())
            {
                const bool mine = slot.machineId == _networkManager->GetMyMachineId();
                DrawText(TextFormat("Slot %d - maquina %d - gos %d%s", s, slot.machineId, slot.dogType, mine ? "  (tu)" : ""),
                    40, 100 + s * 30, 20, mine ? YELLOW : WHITE);
            }
            else DrawText(TextFormat("Slot %d - buit", s), 40, 100 + s * 30, 20, GRAY);
        }
        if (_networkManager->IsHost()) DrawText("ENTER: comencar partida", 40, 100 + MAX_PLAYERS * 30 + 20, 20, GREEN);
    }

    EndDrawing();
}

std::array<uint8_t, MAX_PLAYERS> GameManager::BuildSlotInput()
{
    std::array<uint8_t, MAX_PLAYERS> slotInput;
    slotInput.fill(16 << 3); // Neutral axis value (0b00010000 << 3)

    const auto localInput = _inputManager->GetLevelInput(_playerInfo);
    for (int i = 0; i < _playersActive; i++)
    {
        slotInput[_slotOfLocalPlayers[i]] = localInput[i];
    }
    return slotInput;
}