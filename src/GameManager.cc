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
		TryToAddPlayer(PlayerControllerType::KEYBOARD_ARROW);
	}

	if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) ||
		IsKeyPressed(KEY_SPACE))
	{
		TryToAddPlayer(PlayerControllerType::KEYBOARD_WASD);
	}

	if (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_K) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_L) ||
		IsKeyPressed(KEY_U))
	{
		TryToAddPlayer(PlayerControllerType::KEYBOARD_IJKL);
	}

	// Controller shit
}

void GameManager::Render_PlayerSettings(const float deltaTime)
{
	BeginDrawing();
	EndDrawing();
}

void GameManager::TryToAddPlayer(PlayerControllerType type)
{
	if (_playersActive >= LOCAL_PLAYERS) return;
	if (IsControllerTypeInUse(type)) return;

	_playerInfo[_playersActive].isActive = true;
	_playerInfo[_playersActive].controllerType = type;
	_playersActive++;

	printf("Added player %d with controller type %d\n", _playersActive, (int)type);
}

void GameManager::RemovePlayer(uint8_t playerNum)
{
	if (playerNum >= _playersActive) return;

	_playerInfo[playerNum].isActive = false;

	if (playerNum == LOCAL_PLAYERS - 1) return;
	for (int i = playerNum; i < _playersActive - 1; i++)
	{
		_playerInfo[i] = _playerInfo[i + 1];
	}
	_playersActive--;
}

bool GameManager::IsControllerTypeInUse(PlayerControllerType type) const
{
	for (int i = 0; i < _playersActive; i++)
	{
		if (_playerInfo[i].isActive && _playerInfo[i].controllerType == type) return true;
	}

	return false;
}