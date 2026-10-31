#ifndef LEVEL_HH
#define LEVEL_HH

#include <vector>
#include <array>
#include <memory>
#include <fstream>

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
#include "LevelObjects/Objects/LevelObjectUmbrellaTall.hh"

class LevelManager;

class Level {
public:
	Level(LevelManager* levelManager);
	~Level();

	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;

	void Update(const float deltaTime);
	void Render(const float deltaTime) const;
	void RenderBounds(const float deltaTime) const;

	void LoadLevel(const std::string& levelName);
	LevelObjectDog* SpawnDogInHouse(uint8_t houseIndex, uint8_t playerSlot, uint8_t dogType);

	int GetLevelWidth() const { return _levelWidth; }
	int GetLevelHeight() const { return _levelHeight; }

	void UpdateAutotileNeighbors(int x, int y);

	void DigAt(LevelObjectDog* digger, const Vector2& dogFeetPositionLeft, const Vector2& dogFeetPositionRight);

	int GetDogHouseCount() const { return _dogHouses.size(); }

	const std::vector<LevelObject*>& GetNetBodies() const { return _netBodies; }
	const std::vector<LevelObject*>& GetNetBreakableTiles() const { return _breakableTiles; }

	bool ManagerCanApplyBreak(const LevelObject* cause) const;
	void ManagerNotifyScore(const LevelObject* cause, int points);

protected:

private:
	void InstantiateLevelObject(std::unique_ptr<LevelObject> obj);
	void InstantiateLevelTile(std::unique_ptr<LevelObjectTile> tile, int i, int j);

	void UpdateTileAutotile(int x, int y);

	LevelObjectTileType GetTileTypeAt(const Vector2& position) const;
	LevelObjectTileType GetTileTypeAt(int x, int y) const;

	uint8_t  ComputeRawLowMask(int x, int y) const;
	uint8_t  ComputeRawHighMask(int x, int y) const;
	uint16_t ComputeFullMask(int x, int y) const;
	uint8_t  NormalizeLowMask(uint8_t low) const;
	uint16_t NormalizeFullMask(uint16_t mask) const;
	uint8_t  CollapseHighByte(uint8_t low, uint8_t high) const;
	uint16_t ComputeSlopeVariationMask(uint8_t maskLow, int x, int y) const;
	uint8_t  ComputeTileMaskBridge(int x, int y) const;

	void SetTileSideCollisionMask(int x, int y);

	bool IsTileSame(int x, int y, LevelObjectTileType type) const;
	bool IsTileSlope(int x, int y) const;

	int _levelWidth = 0;
	int _levelHeight = 0;

	std::vector<std::unique_ptr<LevelObject>> _levelObjects;
	std::vector<std::vector<std::unique_ptr<LevelObjectTile>>> _levelTiles;

	std::vector<LevelObjectDog*> _dogs;
	std::vector<LevelObjectDogHouse*> _dogHouses;

	std::vector<LevelObject*> _netBodies;
	std::vector<LevelObject*> _breakableTiles;

	PhysicsManager _physicsManager;
	LevelManager* _levelManager{nullptr};
};

#endif