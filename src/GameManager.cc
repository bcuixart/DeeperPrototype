#include "GameManager.hh"

GameManager* GameManager::instance = nullptr;

GameManager::GameManager()
{
    GameManager::instance = this;

	_assetManager = std::make_unique<AssetManager>();

	_levelManager = std::make_unique<LevelManager>();
	_levelManager->LoadLevel("slope_variation7_test");
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
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int levelWidth = _levelManager->GetLevelWidth();
    int levelHeight = _levelManager->GetLevelHeight();

    float zoomX = (float)screenWidth / levelWidth;
    float zoomY = (float)screenHeight / levelHeight;

    _cam.zoom = std::min(zoomX, zoomY);
    _cam.target = { levelWidth / 2.0f, levelHeight / 2.0f };
    _cam.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    _cam.rotation = 0.0f;

    BeginDrawing();
    BeginMode2D(_cam);
    ClearBackground( {0, 196, 255, 255} );

	_levelManager->Render(deltaTime);
    if (DEBUG_DRAW_BOUNDS) _levelManager->RenderBounds(deltaTime);

    DrawFPS(-500, -500);

    EndMode2D();
    EndDrawing();
}
