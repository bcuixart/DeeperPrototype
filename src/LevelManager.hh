#ifndef LEVELMANAGER_HH
#define LEVELMANAGER_HH

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <iostream>

#include "LevelObject.hh"
#include "LevelObjects/Tiles/LevelObjectTile.hh"
#include "LevelObjects/Tiles/LevelObjectTileGround.hh"

class LevelManager {
public:
	LevelManager();
	~LevelManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	void LoadLevel(const std::string& levelName);

	int GetLevelWidth() const;
	int GetLevelHeight() const;

	LevelObjectTileType GetTileTypeAt(const Vector2& position) const;

protected:

private:

	int _levelWidth = 0;
	int _levelHeight = 0;

	std::vector<std::vector<std::unique_ptr<LevelObjectTile>>> _levelTiles;
};

#endif