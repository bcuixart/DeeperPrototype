#include "GameManager.hh"

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
	case GameState::LEVEL:
		_levelManager->ReceiveInput(_inputManager->GetLevelInput(_playerInfo));
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
		StartLevel("test_level");
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

void GameManager::StartLevel(const std::string& levelName)
{
	if (_playersActive == 0) return;

	_gameState = GameState::LEVEL;

	std::array<uint8_t, MAX_LOCAL_PLAYERS> dogTypes;
	for (int i = 0; i < _playersActive; i++) dogTypes[i] = _playerInfo[i].dogType;

	_levelManager = std::make_unique<LevelManager>(_playersActive);
	_levelManager->LoadLevel(levelName, dogTypes);
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
