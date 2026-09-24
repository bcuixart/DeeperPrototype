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
#include "LevelObjects/Tiles/LevelObjectTileSand.hh"
#include "LevelObjects/Tiles/LevelObjectTileDirt.hh"
#include "LevelObjects/Tiles/LevelObjectTileSlope.hh"
#include "LevelObjects/Tiles/LevelObjectTileBridge.hh"
#include "LevelObjects/Objects/LevelObjectBallTennis.hh"
#include "LevelObjects/Objects/LevelObjectHydrant001.hh"
#include "LevelObjects/Deco/LevelObjectBench001.hh"
#include "LevelObjects/Objects/LevelObjectDogHouse.hh"

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
	LevelObjectTileType GetTileTypeAt(int x, int y) const;

	void UpdateAutotileNeighbors(int x, int y);

protected:

private:
	void InstantiateLevelObject(std::unique_ptr<LevelObject> obj);
	void InstantiateLevelTile(std::unique_ptr<LevelObjectTile> tile, int i, int j);

	void UpdateTileAutotile(int x, int y);

	uint8_t ComputeRawLowMask(int x, int y) const;
	uint8_t ComputeRawHighMask(int x, int y) const;
	uint16_t ComputeFullMask(int x, int y) const;
	uint8_t NormalizeLowMask(uint8_t low) const;
	uint16_t NormalizeFullMask(uint16_t mask) const;
	uint8_t CollapseHighByte(uint8_t low, uint8_t high) const;
	uint16_t ComputeSlopeVariationMask(uint8_t maskLow, int x, int y) const;
	uint8_t ComputeTileMaskSandDirt(int x, int y) const;
	uint8_t ComputeTileMaskBridge(int x, int y) const;

	void SetTileSideCollisionMask(int x, int y);

	bool IsTileSame(int x, int y, LevelObjectTileType type) const;
	bool IsTileSlope(int x, int y) const;

	int _levelWidth = 0;
	int _levelHeight = 0;

	std::unique_ptr<PhysicsManager> _physicsManager;

	std::vector<std::unique_ptr<LevelObject>> _levelObjects;
	std::vector<std::vector<std::unique_ptr<LevelObjectTile>>> _levelTiles;
};

#endif