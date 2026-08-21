#include "LevelManager.hh"

LevelManager::LevelManager()
{

}

LevelManager::~LevelManager()
{
}

void LevelManager::Update(const float deltaTime)
{
}

void LevelManager::Render(const float deltaTime)
{
	for (int i = 0; i < _levelHeight; ++i)
		for (int j = 0; j < _levelWidth; ++j)
			if (_levelTiles[i][j]) _levelTiles[i][j]->Render(deltaTime);
}

void LevelManager::LoadLevel(const std::string& levelName)
{
    std::ifstream file("assets/levels/" + levelName + ".dgl");
    if (!file.is_open()) {
        std::cerr << "[LevelManager] No s'ha pogut obrir el nivell: " << levelName << "\n";
        return;
    }

    file >> _levelWidth >> _levelHeight;
    file.ignore();

    _levelTiles.clear();
    _levelTiles.resize(_levelHeight);
    for (auto& row : _levelTiles)
        row.resize(_levelWidth);

    for (int i = 0; i < _levelHeight; ++i) {
        std::string row;
        std::getline(file, row);

        row.resize(_levelWidth, ' ');

        for (int j = 0; j < _levelWidth; ++j) {
            Vector2 pos = { static_cast<float>(j), static_cast<float>(i) };
            switch (row[j]) {
            case 'G': _levelTiles[i][j] = std::make_unique<LevelObjectTileGround>(pos); break;
            default:  break;
            }
        }
    }
}

int LevelManager::GetLevelWidth() const
{
	return _levelWidth;
}

int LevelManager::GetLevelHeight() const
{
	return _levelHeight;
}

LevelObjectTileType LevelManager::GetTileTypeAt(const Vector2& position) const
{
	int x = static_cast<int>(position.x);
	int y = static_cast<int>(position.y);

	if (x >= 0 && x < _levelWidth && y >= 0 && y < _levelHeight && _levelTiles[y][x])
		return _levelTiles[y][x]->GetTileType();

	return LevelObjectTileType::None;
}