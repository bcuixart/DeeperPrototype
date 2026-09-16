#include "LevelManager.hh"

LevelManager::LevelManager()
{
    _physicsManager = std::make_unique<PhysicsManager>();
}

LevelManager::~LevelManager()
{
    _physicsManager.reset();
}

void LevelManager::Update(const float deltaTime)
{
    for (auto& object : _levelObjects) object->Update(deltaTime);

    _physicsManager->Update(deltaTime, *this);
}

void LevelManager::Render(const float deltaTime) const
{
    for (int i = 0; i < _levelHeight; ++i)
    {
        for (int j = 0; j < _levelWidth; ++j)
        {
            if (_levelTiles[i][j])
            {
                _levelTiles[i][j]->Render(deltaTime);
            }
        }
    }

    for (auto& object : _levelObjects) object->Render(deltaTime);
}

void LevelManager::RenderBounds(const float deltaTime) const
{
    for (int i = 0; i < _levelHeight; ++i)
    {
        for (int j = 0; j < _levelWidth; ++j)
        {
            if (_levelTiles[i][j])
            {
                _levelTiles[i][j]->RenderBounds(deltaTime, BLACK);

                //DrawTextEx(GetFontDefault(), TextFormat("%d", _levelTiles[i][j]->GetSpriteIndex()), { (float)j, (float)i }, 0.5f, 0.025f, WHITE);
                DrawTextEx(GetFontDefault(), TextFormat("%x", NormalizeFullMask(ComputeFullMask(j, i))), { (float)j, (float)i }, 0.5f, 0.025f, WHITE);
            }
        }
    }

    for (auto& object : _levelObjects) object->RenderBounds(deltaTime, ORANGE);
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
            case 'G': InstantiateLevelTile(std::make_unique<LevelObjectTileGround>(pos), i, j); break;
            case 's': InstantiateLevelTile(std::make_unique<LevelObjectTileSlope>(pos), i, j); break;
            case 'B': InstantiateLevelTile(std::make_unique<LevelObjectTileBridge>(pos), i, j); break;
			case 'D': InstantiateLevelObject(std::make_unique<LevelObjectDog>(pos)); break;
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

void LevelManager::InstantiateLevelObject(std::unique_ptr<LevelObject> obj)
{
    LevelObject* raw = obj.get();
    _levelObjects.push_back(std::move(obj));
    _physicsManager->RegisterObject(raw);   
}

void LevelManager::InstantiateLevelTile(std::unique_ptr<LevelObjectTile> tile, int i, int j)
{
    LevelObject* raw = tile.get();
    _levelTiles[i][j] = std::move(tile);
    _physicsManager->RegisterObject(raw);
}

void LevelManager::UpdateTileAutotile(int x, int y)
{
    LevelObjectTileType type = _levelTiles[y][x]->GetTileType();
    switch(type)
    {
        case LevelObjectTileType::None:
            return;

        case LevelObjectTileType::Ground:
        case LevelObjectTileType::Slope:
        case LevelObjectTileType::Sand:
        case LevelObjectTileType::Dirt:
        {
            uint16_t normalized = NormalizeFullMask(ComputeFullMask(x, y));

            auto it = groundAutotileMap.find(normalized);
            _levelTiles[y][x]->SetSpriteIndex(it != groundAutotileMap.end() ? it->second : 0);
            break;
        }
        case LevelObjectTileType::Bridge:
        {
            uint8_t bridgeMask = ComputeTileMaskBridge(x, y);

            auto itBridge = bridgeAutotileMap.find(bridgeMask);
            _levelTiles[y][x]->SetSpriteIndex(itBridge != bridgeAutotileMap.end() ? itBridge->second : 0);  
            break;
        }
        default:
            return;
    }
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

uint16_t LevelManager::ComputeFullMask(int x, int y) const
{
    LevelObjectTileType type = _levelTiles[y][x]->GetTileType();

    uint8_t low = 0, high = 0;
    if (IsTileSame(x - 1, y - 1, type)) { low |= 1; if (IsTileSlope(x-1,y-1)) high |= 1; }      // NW
    if (IsTileSame(x, y - 1, type)) { low |= 2; if (IsTileSlope(x,y-1)) high |= 2; }            // N
    if (IsTileSame(x + 1, y - 1, type)) { low |= 4; if (IsTileSlope(x+1,y-1)) high |= 4; }      // NE
    if (IsTileSame(x - 1, y, type)) { low |= 8; if (IsTileSlope(x-1,y)) high |= 8; }            // W
    if (IsTileSame(x + 1, y, type)) { low |= 16; if (IsTileSlope(x+1,y)) high |= 16; }          // E
    if (IsTileSame(x - 1, y + 1, type)) { low |= 32; if (IsTileSlope(x-1,y+1)) high |= 32; }    // SW
    if (IsTileSame(x, y + 1, type)) { low |= 64; if (IsTileSlope(x,y+1)) high |= 64; }         // S
    if (IsTileSame(x + 1, y + 1, type)) { low |= 128; if (IsTileSlope(x+1,y+1)) high |= 128; } // SE

    return (uint16_t)(low | (high << 8));
}

uint16_t LevelManager::NormalizeFullMask(uint16_t mask) const
{
    uint8_t low  = mask & 0xFF;
    uint8_t high = (mask >> 8) & 0xFF;

    if (!(low & 2) || !(low & 8)) low &= ~1;     // NW  needs N & W
    if (!(low & 2) || !(low & 16)) low &= ~4;    // NE  needs N & E
    if (!(low & 64) || !(low & 8)) low &= ~32;   // SW  needs S & W
    if (!(low & 64) || !(low & 16)) low &= ~128; // SE  needs S & E

    high &= 0b01011010;                           // Eliminate corners that are not needed for slopes
    for (int b = 0; b < 8; ++b)
        if (!(low & (1 << b))) high &= ~(1 << b); // Eliminate slope from a corner if the corresponding cardinal is not present

    high = CollapseHighByte(low, high);

    return (uint16_t)(low | (high << 8));
}

uint8_t LevelManager::CollapseHighByte(uint8_t low, uint8_t high) const
{
    switch (low) 
    {
        case 80:  case 120: case 127: case 122:
        case 82:  case 126: case 35:  case 86:
            if (high & 16) { high |= 64; high &= ~16; }
            break;
        case 72:  case 216: case 223: case 75:
        case 74:  case 222: case 36:  case 219: case 123:
            if (high & 8)  { high |= 64; high &= ~8;  }
            break;
        default: break;
    }
    return high;
}

uint8_t LevelManager::ComputeTileMaskBridge(int x, int y) const
{
    auto same = [&](int cx, int cy) -> bool {
        if (cx < 0 || cx >= _levelWidth || cy < 0 || cy >= _levelHeight || !_levelTiles[cy][cx])
            return false;
        return _levelTiles[cy][cx]->GetTileType() == LevelObjectTileType::Bridge;
        };

    uint8_t mask = 0;
    if (same(x - 1, y)) mask |= 1;          // W
    if (same(x + 1, y)) mask |= 2;          // E

    return mask;
}

bool LevelManager::IsTileSame(int x, int y, LevelObjectTileType type) const
{
    if (x < 0 || x >= _levelWidth || y < 0 || y >= _levelHeight) return false;
    if (!_levelTiles[y][x]) return false;

    switch(type)
    {
        case LevelObjectTileType::None:
            return false;

        case LevelObjectTileType::Ground:
        case LevelObjectTileType::Slope:
        {
            return _levelTiles[y][x]->GetTileType() == LevelObjectTileType::Ground || _levelTiles[y][x]->GetTileType() == LevelObjectTileType::Slope;
            break;
        }
        case LevelObjectTileType::Sand:
        case LevelObjectTileType::Dirt:
        {
            return _levelTiles[y][x]->GetTileType() == type;  
        }
        case LevelObjectTileType::Bridge:
        {
            return _levelTiles[y][x]->GetTileType() == type;  
        }
        default:
            return false;
    }

    return false;
}

bool LevelManager::IsTileSlope(int x, int y) const
{
    if (x < 0 || x >= _levelWidth || y < 0 || y >= _levelHeight) return false;
    if (!_levelTiles[y][x]) return false;

    return _levelTiles[y][x]->GetTileType() == LevelObjectTileType::Slope;
}