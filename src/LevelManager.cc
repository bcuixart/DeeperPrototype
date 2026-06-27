#include "LevelManager.hh"

LevelManager::LevelManager()
{
	for (int i = 0; i < LEVEL_HEIGHT; ++i)
	{
		for (int j = 0; j < LEVEL_WIDTH; ++j)
		{
			char tileChar = _testLevel[i][j];
			if (tileChar == 'G')
			{
				Vector2 position = { static_cast<float>(j - 5), static_cast<float>(i - 5) };
				_levelTiles[i][j] = std::make_unique<LevelObjectTileGround>(position);
			}
		}
	}
}

LevelManager::~LevelManager()
{
}

void LevelManager::Update(const float deltaTime)
{
}

void LevelManager::Render(const float deltaTime)
{
	for (int i = 0; i < LEVEL_HEIGHT; ++i)
	{
		for (int j = 0; j < LEVEL_WIDTH; ++j)
		{
			if (_levelTiles[i][j]) _levelTiles[i][j]->Render(deltaTime);
		}
	}
}

LevelObjectTileType LevelManager::GetTileTypeAt(const Vector2& position) const
{
	int x = static_cast<int>(position.x);
	int y = static_cast<int>(position.y);
	
	if (x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT)
	{
		if (_levelTiles[y][x])
		{
			return _levelTiles[y][x]->GetTileType();
		}
	}

	return LevelObjectTileType::None;
}