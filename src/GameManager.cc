#include "GameManager.hh"

GameManager* GameManager::instance = nullptr;

GameManager::GameManager()
{
    GameManager::instance = this;
	_gameState = GameState::PLAYER_SETTINGS;

	_assetManager = std::make_unique<AssetManager>();

	for (int i = 0; i < LOCAL_PLAYERS; i++)
	{
		_playerInfo[i].isActive = false;
	}
	_playersActive = 0;

	_menuManager = std::make_unique<MenuManager>();

	_levelManager = std::make_unique<LevelManager>();
	_levelManager->LoadLevel("test_level");
}

GameManager::~GameManager()
{
    GameManager::instance = nullptr;

    _assetManager.reset();
	_levelManager.reset();
}

void GameManager::Update(const float deltaTime)
{
	switch (_gameState)
	{
	case GameState::MAIN_MENU:
		break;
	case GameState::PLAYER_SETTINGS:
		Update_PlayerSettings(deltaTime);
		_menuManager->Update(deltaTime);
		break;
	case GameState::PLAYING:
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
		Render_PlayerSettings(deltaTime);
		_menuManager->Render(deltaTime);
		break;
	case GameState::PLAYING:
		_levelManager->Render(deltaTime);
		break;
	}
}

void GameManager::Update_PlayerSettings(const float deltaTime)
{
	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
		IsKeyPressed(KEY_LEFT_CONTROL))
	{
		TryToAddPlayer(PlayerControllerType::KEYBOARD_ARROW, 0);
	}

	if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) ||
		IsKeyPressed(KEY_SPACE))
	{
		TryToAddPlayer(PlayerControllerType::KEYBOARD_WASD, 0);
	}

	if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_K) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_L) ||
		IsKeyPressed(KEY_U))
	{
		TryToAddPlayer(PlayerControllerType::KEYBOARD_IJKL, 0);
	}

	for (int gamepadIndex = 0; gamepadIndex < MAX_GAMEPADS; gamepadIndex++)
	{
		if (!IsGamepadAvailable(gamepadIndex)) continue;
		if (AnyGamepadButtonPressed(gamepadIndex))
		{
			TryToAddPlayer((PlayerControllerType)((int)PlayerControllerType::GAMEPAD_0 + gamepadIndex), gamepadIndex);
		}
	}

	CheckForDisconnectedGamepadPlayers();
}

void GameManager::Render_PlayerSettings(const float deltaTime)
{
	BeginDrawing();
	EndDrawing();
}

bool GameManager::TryToAddPlayer(PlayerControllerType type, uint8_t gamepadIndex)
{
	if (_playersActive >= LOCAL_PLAYERS) return false;
	if (IsControllerTypeInUse(type)) return false;

	_playerInfo[_playersActive].isActive = true;
	_playerInfo[_playersActive].controllerType = type;
	_playerInfo[_playersActive].gamepadIndex = gamepadIndex;
	_playersActive++;

	printf("Added player %d with controller type %d\n", _playersActive, (int)type);

	return true;
}

bool GameManager::TryToRemovePlayer(uint8_t playerNum)
{
	if (playerNum >= _playersActive) return false;
	if (!_playerInfo[playerNum].isActive) return false;

	_playerInfo[playerNum].isActive = false;

	if (playerNum == LOCAL_PLAYERS - 1) return true;
	for (int i = playerNum; i < _playersActive - 1; i++)
	{
		_playerInfo[i] = _playerInfo[i + 1];
	}
	_playersActive--;

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

bool GameManager::AnyGamepadButtonPressed(int gamepadIndex) const
{
	// GAMEPAD_BUTTON_UNKNOWN (0) is excluded
	for (int button = GAMEPAD_BUTTON_LEFT_FACE_UP; button <= GAMEPAD_BUTTON_RIGHT_THUMB; button++)
	{
		if (IsGamepadButtonPressed(gamepadIndex, button)) return true;
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