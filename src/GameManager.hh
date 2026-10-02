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
	GameState _gameState;

	std::unique_ptr<AssetManager> _assetManager = nullptr;
	std::unique_ptr<LevelManager> _levelManager = nullptr;
};

#endif