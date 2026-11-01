#ifndef LEVEL_HH
#define LEVEL_HH

#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <algorithm>

#include "PhysicsManager.hh"
#include "LevelObjectsList.hh"

constexpr int kMaxLevelWidth = 64;
constexpr int kMaxLevelHeight = 32;

class LevelManager;

class Level {
public:
	Level(LevelManager* levelManager);
	~Level();

	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;

	void Update(const float deltaTime);

	void RenderBackground(const float deltaTime) const;
	void RenderObjects(const float deltaTime) const;
	void RenderBounds(const float deltaTime) const;
	void RenderEditorGrid(const float deltaTime) const;
	void RenderEditorGridBounds(const float deltaTime) const;
	void RenderEditorObjectBounds(const float deltaTime) const;
	void RenderEditorDraggingPreview(const float deltaTime, uint8_t objectId, int x, int y) const;

	Camera2D FitCameraToBounds(float margin, float minWidthTiles, float minHeightTiles) const;

	void ReadLevelFromFile(const std::string& fileName);
	void BuildLevelFromStream(std::istream& stream);
	LevelObjectDog* SpawnDogInHouse(uint8_t houseIndex, uint8_t playerSlot, uint8_t dogType);

	bool EditorTryToPlaceObject(int x, int y, uint8_t objectId);
	bool EditorTryToRemoveObject(const Vector2& position, int x, int y);

	void UpdateAutotileNeighbors(int x, int y, int radius);

	void DigAt(LevelObjectDog* digger, const Vector2& dogFeetPositionLeft, const Vector2& dogFeetPositionRight);

	int GetDogHouseCount() const { return _dogHouses.size(); }

	const std::vector<LevelObject*>& GetNetBodies() const { return _netBodies; }
	const std::vector<LevelObject*>& GetNetBreakableTiles() const { return _breakableTiles; }

	bool ManagerCanApplyBreak(const LevelObject* cause) const;
	void ManagerNotifyScore(const LevelObject* cause, int points);

protected:

private:
	struct Placement
	{
		uint8_t id;
		Vector2 placedPosition;
		Rectangle editorBounds;
		std::vector<LevelObject*> parts;
	};

	bool PlaceObjectById(uint8_t id, int x, int y);

	template <typename T> T* AddSingleObject(uint8_t id, const Vector2& pos, const Rectangle& editorBounds);
	void AddPlacement(uint8_t id, const Vector2& placedPosition, const Rectangle& editorBounds, std::vector<std::unique_ptr<LevelObject>>&& parts);
	LevelObjectTile* AddTile(uint8_t id, std::unique_ptr<LevelObjectTile> tile, int x, int y);
	void AddDog(std::unique_ptr<LevelObjectDog> dog, uint8_t playerSlot, uint8_t dogType);

	void DeleteLevelObject(size_t placementIndex);
	void DeleteLevelTile(int x, int y);

	void RemoveFromViews(LevelObject* obj);

	bool ComputeEditorBoundsForObject(uint8_t id, int x, int y, Rectangle& outBounds) const;
	bool CanBoundsBePlacedAt(const Rectangle& editorBounds) const;

	void ComputeLevelBounds();

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

	Rectangle _levelBounds = { 0, 0, 0, 0 };

	std::vector<std::unique_ptr<LevelObject>> _levelObjects;
	std::array<std::array<std::unique_ptr<LevelObjectTile>, kMaxLevelWidth>, kMaxLevelHeight> _levelTiles;

	std::vector<Placement> _objectPlacements;
	std::array<std::array<uint8_t, kMaxLevelWidth>, kMaxLevelHeight> _tileIds{};

	std::vector<LevelObjectDog*> _dogs;
	std::vector<LevelObjectDogHouse*> _dogHouses;

	std::vector<LevelObject*> _netBodies;
	std::vector<LevelObject*> _breakableTiles;

	PhysicsManager _physicsManager;
	LevelManager* _levelManager{nullptr};
};

#endif