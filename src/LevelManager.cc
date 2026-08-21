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

    for (int y = 0; y < _levelHeight; ++y)
        for (int x = 0; x < _levelWidth; ++x)
            if (_levelTiles[y][x]) UpdateTileAutotile(x, y);
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

void LevelManager::UpdateTileAutotile(int x, int y)
{
    uint8_t normalized = NormalizeTileRawMask(ComputeTileRawMask(x, y));

    auto it = groundAutotileMap.find(normalized);
    _levelTiles[y][x]->SetSpriteIndex(it != groundAutotileMap.end() ? it->second : 0);
}

void LevelManager::UpdateAutotileNeighbors(int x, int y)
{
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx) 
        {
            int nx = x + dx; 
            int ny = y + dy;
            if (nx >= 0 && nx < _levelWidth && ny >= 0 && ny < _levelHeight)
            {
                if (_levelTiles[ny][nx]) UpdateTileAutotile(nx, ny);
            }
        }
    }
}

uint8_t LevelManager::ComputeTileRawMask(int x, int y) const
{
    LevelObjectTileType type = _levelTiles[y][x]->GetTileType();

    auto same = [&](int cx, int cy) -> bool {
        if (cx < 0 || cx >= _levelWidth || cy < 0 || cy >= _levelHeight)
            return false;
        return _levelTiles[cy][cx] && _levelTiles[cy][cx]->GetTileType() == type;
        };

    uint8_t mask = 0;
    if (same(x - 1, y - 1)) mask |= 1;      // NW
    if (same(x, y - 1)) mask |= 2;          // N
    if (same(x + 1, y - 1)) mask |= 4;      // NE
    if (same(x - 1, y)) mask |= 8;          // W
    if (same(x + 1, y)) mask |= 16;         // E
    if (same(x - 1, y + 1)) mask |= 32;     // SW
    if (same(x, y + 1)) mask |= 64;         // S
    if (same(x + 1, y + 1)) mask |= 128;    // SE
    return mask;
}

uint8_t LevelManager::NormalizeTileRawMask(uint8_t mask) const
{
    if (!(mask & 2) || !(mask & 8)) mask &= ~1;     // NW  needs N & W
    if (!(mask & 2) || !(mask & 16)) mask &= ~4;    // NE  needs N & E
    if (!(mask & 64) || !(mask & 8)) mask &= ~32;   // SW  needs S & W
    if (!(mask & 64) || !(mask & 16)) mask &= ~128; // SE  needs S & E
    return mask;
}