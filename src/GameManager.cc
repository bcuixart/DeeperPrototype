#include "GameManager.hh"

GameManager* GameManager::instance = nullptr;

GameManager::GameManager()
{
    GameManager::instance = this;

	_levelManager = std::make_unique<LevelManager>();
}

GameManager::~GameManager()
{
    GameManager::instance = nullptr;

	_levelManager.reset();
}

void GameManager::Update(const float deltaTime)
{
	_levelManager->Update(deltaTime);
}

void GameManager::Render(const float deltaTime) 
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    _cam.target = { 0, 0 };
    _cam.offset = { width / 2.0f, height / 2.0f }; // Screen center
    _cam.rotation = 0.0f;
    _cam.zoom = (float)height / 10.0f;

    BeginDrawing();
    BeginMode2D(_cam);
    ClearBackground( {0, 196, 255, 255} );

	_levelManager->Render(deltaTime);

    DrawFPS(-500, -500);

    EndMode2D();
    EndDrawing();
}
