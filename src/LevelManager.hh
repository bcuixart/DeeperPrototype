#ifndef LEVELMANAGER_HH
#define LEVELMANAGER_HH

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <iostream>

#include "PhysicsManager.hh"
#include "LevelObject.hh"
#include "LevelObjects/LevelObjectDog.hh"
#include "LevelObjects/Tiles/Autotile.hh"
#include "LevelObjects/Tiles/LevelObjectTile.hh"
#include "LevelObjects/Tiles/LevelObjectTileGround.hh"
#include "LevelObjects/Tiles/LevelObjectTileSlope.hh"

class LevelManager {
public:
	LevelManager();
	~LevelManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime) const;
	void RenderBounds(const float deltaTime) const;

	void LoadLevel(const std::string& levelName);

	int GetLevelWidth() const;
	int GetLevelHeight() const;

	LevelObjectTileType GetTileTypeAt(const Vector2& position) const;

protected:

private:
	void InstantiateLevelObject(std::unique_ptr<LevelObject> obj);
	void InstantiateLevelTile(std::unique_ptr<LevelObjectTile> tile, int i, int j);

	void UpdateTileAutotile(int x, int y);
	void UpdateAutotileNeighbors(int x, int y);

	uint8_t ComputeTileRawMask(int x, int y) const;
	uint8_t NormalizeTileRawMask(uint8_t mask) const;

	int _levelWidth = 0;
	int _levelHeight = 0;

	std::unique_ptr<PhysicsManager> _physicsManager;

	std::vector<std::unique_ptr<LevelObject>> _levelObjects;
	std::vector<std::vector<std::unique_ptr<LevelObjectTile>>> _levelTiles;
};

#endif