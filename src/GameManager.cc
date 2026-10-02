#include "GameManager.hh"

GameManager* GameManager::instance = nullptr;

GameManager::GameManager()
{
    GameManager::instance = this;
	_gameState = GameState::PLAYER_SETTINGS;

	_assetManager = std::make_unique<AssetManager>();

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
	_levelManager->Update(deltaTime);
}

void GameManager::Render(const float deltaTime) 
{
	_levelManager->Render(deltaTime);
}
