#ifndef LEVELMANAGER_HH
#define LEVELMANAGER_HH

#include <vector>
#include <array>
#include <memory>

#include "LevelObject.hh"
#include "LevelObjects/Tiles/LevelObjectTile.hh"
#include "LevelObjects/Tiles/LevelObjectTileGround.hh"

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 10

class LevelManager {
public:
	LevelManager();
	~LevelManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime);

	LevelObjectTileType GetTileTypeAt(const Vector2& position) const;

protected:

private:
	const char _testLevel[LEVEL_HEIGHT][LEVEL_WIDTH + 1] =
	{
		"GGGGGGGGGG",
		"G********G",
		"G**G*****G",
		"G********G",
		"G********G",
		"G******G*G",
		"G********G",
		"G****G***G",
		"GG*******G",
		"GGGGGGGGGG",
	};

	std::array<std::array<std::unique_ptr<LevelObjectTile>, LEVEL_WIDTH>, LEVEL_HEIGHT> _levelTiles{};
};

#endif