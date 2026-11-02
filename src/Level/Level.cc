#include "Level.hh"
#include "LevelManager.hh"

Level::Level(LevelManager* levelManager)
    : _levelManager(levelManager)
{
    _physicsManager = PhysicsManager();
}

Level::~Level()
{

}

void Level::Update(const float deltaTime)
{
    for (auto& object : _levelObjects) object->Update(deltaTime);

    _physicsManager.Update(deltaTime);
}

void Level::RenderBackground(const float deltaTime) const
{
    ClearBackground({ 0, 196, 255, 255 });
}

void Level::RenderObjects(const float deltaTime) const
{
    for (int i = 0; i < kMaxLevelHeight; ++i)
    {
        for (int j = 0; j < kMaxLevelWidth; ++j)
        {
            if (_levelTiles[i][j])
            {
                _levelTiles[i][j]->Render(deltaTime);
            }
        }
    }

    for (auto& object : _levelObjects) object->Render(deltaTime);
}

void Level::RenderBounds(const float deltaTime) const
{
    for (int i = 0; i < kMaxLevelHeight; ++i)
    {
        for (int j = 0; j < kMaxLevelWidth; ++j)
        {
            if (_levelTiles[i][j])
            {
                _levelTiles[i][j]->RenderBounds(deltaTime, RED);

                //DrawTextEx(GetFontDefault(), TextFormat("%d", _levelTiles[i][j]->GetSpriteIndex()), { (float)j, (float)i }, 0.5f, 0.025f, WHITE);
                //DrawTextEx(GetFontDefault(), TextFormat("%x", NormalizeFullMask(ComputeFullMask(j, i))), { (float)j, (float)i }, 0.5f, 0.025f, WHITE);
            }
        }
    }

    for (auto& object : _levelObjects) object->RenderBounds(deltaTime, ORANGE);
}

void Level::RenderEditorGrid(const float deltaTime) const
{
    for (int i = 0; i <= kMaxLevelHeight; ++i)
    {
        DrawLine(0, i, kMaxLevelWidth, i, { 255, 255, 255, 64 });
    }
    for (int j = 0; j <= kMaxLevelWidth; ++j)
    {
        DrawLine(j, 0, j, kMaxLevelHeight, { 255, 255, 255, 64 });
    }
}

void Level::RenderEditorGridBounds(const float deltaTime) const
{
    DrawRectangleLinesEx(_levelBounds, 0.05f, GREEN);
}

void Level::RenderEditorObjectBounds(const float deltaTime) const
{
    for (int i = 0; i < kMaxLevelHeight; ++i)
    {
        for (int j = 0; j < kMaxLevelWidth; ++j)
        {
            if (_levelTiles[i][j])
            {
                _levelTiles[i][j]->RenderBounds(deltaTime, RED);
            }
        }
    }

    for (const Placement& placement : _objectPlacements)
    {
        DrawRectangleLinesEx(placement.editorBounds, 0.05f, BLUE);
    }
}

void Level::RenderEditorDraggingPreview(const float deltaTime, uint8_t objectId, int x, int y) const
{
    Rectangle bounds;
    if (!ComputeEditorBoundsForObject(objectId, x, y, bounds)) return;

    if (CanBoundsBePlacedAt(bounds)) DrawRectangleLinesEx(bounds, 0.05f, GREEN);
    else DrawRectangleLinesEx(bounds, 0.05f, RED);
}

Camera2D Level::FitCameraToBounds(float margin, float minWidthTiles, float minHeightTiles) const
{
    const float w = std::max(_levelBounds.width  + 2.0f * margin, minWidthTiles);
    const float h = std::max(_levelBounds.height + 2.0f * margin, minHeightTiles);

    Camera2D cam = { 0 };
    cam.zoom     = std::min((float)GetScreenWidth() / w, (float)GetScreenHeight() / h);
    cam.target   = { _levelBounds.x + _levelBounds.width / 2.0f, _levelBounds.y + _levelBounds.height / 2.0f };
    cam.offset   = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    cam.rotation = 0.0f;

    return cam;
}

bool Level::BuildLevelFromFile(const std::string& fileName)
{
    std::ifstream file("assets/levels/" + fileName + ".dgl");
    if (!file.is_open())
    {
        std::cerr << "[Level] No s'ha pogut obrir el nivell: " << fileName << "\n";
        return false;
    }
    return BuildLevelFromStream(file);
}

bool Level::BuildLevelFromStream(std::istream& stream)
{
    int width, height;
    stream >> width >> height;
    stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (!stream || width <= 0 || height <= 0 || width > kMaxLevelWidth || height > kMaxLevelHeight)
    {
        std::cerr << "[Level] Mida de nivell invàlida\n";
        return false;
    }

    _levelObjects.clear();
    for (auto& row : _levelTiles) for (auto& t : row) t.reset();
    _dogs.clear();
    _dogHouses.clear();
    _netBodies.clear();
    _breakableTiles.clear();
    _physicsManager = PhysicsManager();

    _objectPlacements.clear();
    _tileIds = {};

    for (int y = 0; y < kMaxLevelHeight; ++y)
    {
        std::string row;
        std::getline(stream, row);

        row.resize(kMaxLevelWidth, ' ');

        for (int x = 0; x < kMaxLevelWidth; ++x) PlaceObjectById((uint8_t)row[x], x, y); 
    }

    for (int y = 0; y < kMaxLevelHeight; ++y)
        for (int x = 0; x < kMaxLevelWidth; ++x)
            if (_levelTiles[y][x]) UpdateTileAutotile(x, y);

    for (int y = 0; y < kMaxLevelHeight; ++y)
        for (int x = 0; x < kMaxLevelWidth; ++x)
            if (_levelTiles[y][x]) SetTileSideCollisionMask(x, y);

    ComputeLevelBounds();

    return true;
}

void Level::WriteLevelToStream(std::ostream& out) const
{
    out << kMaxLevelWidth << " " << kMaxLevelHeight << "\n";

    std::array<std::string, kMaxLevelHeight> rows;
    for (auto& r : rows) r.assign(kMaxLevelWidth, '.');

    for (int y = 0; y < kMaxLevelHeight; ++y)
        for (int x = 0; x < kMaxLevelWidth; ++x)
            if (_tileIds[y][x]) rows[y][x] = (char)_tileIds[y][x];

    for (const Placement& p : _objectPlacements)
        rows[(int)p.placedPosition.y][(int)p.placedPosition.x] = (char)p.id;

    for (const auto& r : rows) out << r << '\n';
}

bool Level::PlaceObjectById(uint8_t id, int x, int y)
{
    auto it = levelObjectsList.find(id);
    if (it == levelObjectsList.end()) return false;
    
    Rectangle editorBounds;
    editorBounds = { x + it->second.editorBounds.x, y + it->second.editorBounds.y, it->second.editorBounds.width, it->second.editorBounds.height };

    const Vector2 pos = { (float)x, (float)y };

    switch (id)
    {
        // --- Tiles ---
        case kIdGround: AddTile(id, std::make_unique<LevelObjectTileGround>(pos), x, y); break;
        case kIdSlope:  AddTile(id, std::make_unique<LevelObjectTileSlope>(pos), x, y);  break;
        case kIdDirt:   AddTile(id, std::make_unique<LevelObjectTileDirt>(pos), x, y);   break;
        case kIdBridge: AddTile(id, std::make_unique<LevelObjectTileBridge>(pos), x, y); break;
        case kIdSand:
            _breakableTiles.push_back(AddTile(id, std::make_unique<LevelObjectTileSand>(pos, this), x, y));
            break;

        // --- Objects ---
        case kIdTennisBall:   AddSingleObject<LevelObjectBallTennis>(id, pos, editorBounds);   break;
        case kIdBench:        AddSingleObject<LevelObjectBench001>(id, pos, editorBounds);     break;
        case kIdUmbrellaTall: AddSingleObject<LevelObjectUmbrellaTall>(id, pos, editorBounds); break;
        case kIdDogHouse:
            _dogHouses.push_back(AddSingleObject<LevelObjectDogHouse>(id, pos, editorBounds));
            break;
        case kIdHydrant:
        {
            auto waterLeft  = std::make_unique<LevelObjectHydrant001Water>(pos, true);
            auto waterRight = std::make_unique<LevelObjectHydrant001Water>(pos, false);
            auto* wl = waterLeft.get();
            auto* wr = waterRight.get();

            std::vector<std::unique_ptr<LevelObject>> parts;
            parts.push_back(std::move(waterLeft));
            parts.push_back(std::move(waterRight));
            parts.push_back(std::make_unique<LevelObjectHydrant001>(pos, wl, wr));
            AddPlacement(id, pos, editorBounds, std::move(parts));
            break;
        }
        default: return false;
    }
    return true;
}

LevelObjectDog* Level::SpawnDogInHouse(uint8_t houseIndex, uint8_t playerSlot, uint8_t dogType)
{
    if (houseIndex >= _dogHouses.size()) return nullptr;

    Vector2 dogPos = { _dogHouses[houseIndex]->GetPosition().x, _dogHouses[houseIndex]->GetPosition().y };

    auto dog = std::make_unique<LevelObjectDog>(dogPos, this);
    auto raw = dog.get();
    AddDog(std::move(dog), playerSlot, dogType);

    return raw;
}

void Level::DigAt(LevelObjectDog* digger, const Vector2& dogFeetPositionLeft, const Vector2& dogFeetPositionRight)
{
    LevelObject* dugObjectLeft = _physicsManager.GetDugObjectAt(dogFeetPositionLeft, digger);
    LevelObject* dugObjectRight = _physicsManager.GetDugObjectAt(dogFeetPositionRight, digger);

    if (dugObjectLeft != nullptr && dugObjectRight != nullptr)
    {
        if (dugObjectLeft == dugObjectRight) dugObjectLeft->OnDug(digger);
        else
        {
            // He de pensar com resoldre aixo.
            // Si els dos son semisolids, d'entrada cap problema
            // Pero si un es terra i l'altre no?
            // Hi ha moltes combinacions possibles, i no se si es correcte que el gos pugui cavar a terra i a un semisolid alhora.
            dugObjectLeft->OnDug(digger);
            dugObjectRight->OnDug(digger);
        }
    }
    else if (dugObjectLeft != nullptr) dugObjectLeft->OnDug(digger);
    else if (dugObjectRight != nullptr) dugObjectRight->OnDug(digger);
}

LevelObjectTileType Level::GetTileTypeAt(const Vector2& position) const
{
	int x = static_cast<int>(position.x);
	int y = static_cast<int>(position.y);

	return GetTileTypeAt(x, y);
}

LevelObjectTileType Level::GetTileTypeAt(int x, int y) const
{
	if (x >= 0 && x < kMaxLevelWidth && y >= 0 && y < kMaxLevelHeight && _levelTiles[y][x])
		return _levelTiles[y][x]->GetTileType();

	return LevelObjectTileType::None;
}

bool Level::EditorTryToPlaceObject(int x, int y, uint8_t objectId)
{
    if (x < 0 || x >= kMaxLevelWidth || y < 0 || y >= kMaxLevelHeight) return false;

    Rectangle bounds;
    if (!ComputeEditorBoundsForObject(objectId, x, y, bounds)) return false;
    if (!CanBoundsBePlacedAt(bounds)) return false;

    if (!PlaceObjectById(objectId, x, y)) return false;
    
    if (levelObjectsList.at(objectId).category == LevelObjectCategory::Tile) UpdateAutotileNeighbors(x, y, 2);

    ComputeLevelBounds();

    return true;
}

bool Level::EditorTryToRemoveObject(const Vector2& mousePosition, int x, int y)
{
    // Check if the position is within the level bounds
    if (mousePosition.x < 0 || mousePosition.x >= kMaxLevelWidth || mousePosition.y < 0 || mousePosition.y >= kMaxLevelHeight) return false;
    if (x < 0 || x >= kMaxLevelWidth || y < 0 || y >= kMaxLevelHeight) return false;

    // First try to delete objects
    for (size_t i = 0; i < _objectPlacements.size(); ++i)
        if (CheckCollisionPointRec(mousePosition, _objectPlacements[i].editorBounds))
        {
            DeleteLevelObject(i);
            return true;
        }

    if (_levelTiles[y][x]) 
    {
        DeleteLevelTile(x, y);
        return true;
    }
    return false;
}

bool Level::CanEnterPlaytest() const
{
    return _dogHouses.size() > 0;
}

template <typename T>
T* Level::AddSingleObject(uint8_t id, const Vector2& pos, const Rectangle& editorBounds)
{
    auto obj = std::make_unique<T>(pos);
    T* raw = obj.get();

    std::vector<std::unique_ptr<LevelObject>> parts;
    parts.push_back(std::move(obj));
    AddPlacement(id, pos, editorBounds, std::move(parts));

    return raw;
}

void Level::AddPlacement(uint8_t id, const Vector2& placedPosition, const Rectangle& editorBounds, std::vector<std::unique_ptr<LevelObject>>&& parts)
{
    Placement p{ id, placedPosition, editorBounds, {} };

    for (auto& part : parts) 
    {
        LevelObject* raw = part.get();
        _levelObjects.push_back(std::move(part));
        _physicsManager.RegisterObject(raw);

        NetworkLevelObjectDynEntState probe;
        if (raw->NetGet(probe)) _netBodies.push_back(raw);

        p.parts.push_back(raw);
    }

    _objectPlacements.push_back(std::move(p));
}

LevelObjectTile* Level::AddTile(uint8_t id, std::unique_ptr<LevelObjectTile> tile, int x, int y)
{
    LevelObjectTile* raw = tile.get();
    _levelTiles[y][x] = std::move(tile);

    _tileIds[y][x] = id;
    _physicsManager.RegisterObject(raw);

    return raw;
}

void Level::AddDog(std::unique_ptr<LevelObjectDog> dog, uint8_t playerSlot, uint8_t dogType)
{
    LevelObjectDog* raw = dog.get();
    _levelObjects.push_back(std::move(dog));
    _physicsManager.RegisterObject(raw);

    _netBodies.push_back(raw);

    raw->SetSlot(playerSlot);
    raw->SetDogType(dogType);

    _dogs.push_back(raw);
}

void Level::DeleteLevelObject(size_t placementIndex)
{
    const std::vector<LevelObject*> objects = _objectPlacements[placementIndex].parts;

    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
    {
        LevelObject* obj = *it;

        _physicsManager.UnregisterObject(obj);
        RemoveFromViews(obj);

        _levelObjects.erase(std::remove_if(_levelObjects.begin(), _levelObjects.end(),
            [obj](const std::unique_ptr<LevelObject>& o) { return o.get() == obj; }),
            _levelObjects.end());
    }

    _objectPlacements.erase(_objectPlacements.begin() + placementIndex);

    ComputeLevelBounds();
}

void Level::DeleteLevelTile(int x, int y)
{
    LevelObjectTile* tile = _levelTiles[y][x].get();
    if (!tile) return;

    _physicsManager.UnregisterObject(tile);
    RemoveFromViews(tile);
    _levelTiles[y][x].reset();
    _tileIds[y][x] = 0;

    UpdateAutotileNeighbors(x, y, 2);
    ComputeLevelBounds();
}

void Level::RemoveFromViews(LevelObject* obj)
{
    auto drop = [obj](auto& v) { v.erase(std::remove(v.begin(), v.end(), obj), v.end()); };
    drop(_netBodies);
    drop(_breakableTiles);
    drop(_dogs);
    drop(_dogHouses);
}

bool Level::ComputeEditorBoundsForObject(uint8_t id, int x, int y, Rectangle& outBounds) const
{
    if (levelObjectsList.find(id) == levelObjectsList.end()) return false;
    LevelObjectProperties properties = levelObjectsList.at(id);

    outBounds = { x + properties.editorBounds.x, y + properties.editorBounds.y, properties.editorBounds.width, properties.editorBounds.height };
    return true;
}

bool Level::CanBoundsBePlacedAt(const Rectangle& editorBounds) const
{
    if (editorBounds.x < 0 || editorBounds.y < 0 || editorBounds.x + editorBounds.width > kMaxLevelWidth || editorBounds.y + editorBounds.height > kMaxLevelHeight) return false;

    for (const Placement& p : _objectPlacements)
        if (CheckCollisionRecs(editorBounds, p.editorBounds)) return false;

    for (int ty = (int)floorf(editorBounds.y); ty < (int)ceilf(editorBounds.y + editorBounds.height); ++ty)
        for (int tx = (int)floorf(editorBounds.x); tx < (int)ceilf(editorBounds.x + editorBounds.width); ++tx)
            if (_levelTiles[ty][tx]) return false;

    return true;
}

void Level::ComputeLevelBounds()
{
    float xMin = (float)kMaxLevelWidth,  xMax = 0.0f;
    float yMin = (float)kMaxLevelHeight, yMax = 0.0f;
    bool any = false;

    for (const Placement& p : _objectPlacements)
    {
        xMin = std::min(xMin, p.editorBounds.x);
        yMin = std::min(yMin, p.editorBounds.y);
        xMax = std::max(xMax, p.editorBounds.x + p.editorBounds.width);
        yMax = std::max(yMax, p.editorBounds.y + p.editorBounds.height);
        any = true;
    }

    for (int y = 0; y < kMaxLevelHeight; ++y)
        for (int x = 0; x < kMaxLevelWidth; ++x)
            if (_levelTiles[y][x])
            {
                xMin = std::min(xMin, (float)x);       yMin = std::min(yMin, (float)y);
                xMax = std::max(xMax, (float)(x + 1)); yMax = std::max(yMax, (float)(y + 1));
                any = true;
            }

    _levelBounds = any ? Rectangle{ xMin, yMin, xMax - xMin, yMax - yMin }
                       : Rectangle{ 0, 0, (float)kMaxLevelWidth, (float)kMaxLevelHeight };
}

void Level::UpdateTileAutotile(int x, int y)
{
    LevelObjectTileType type = _levelTiles[y][x]->GetTileType();
    switch(type)
    {
        case LevelObjectTileType::None:
            return;

        case LevelObjectTileType::Ground:
        {
            uint16_t normalized = NormalizeFullMask(ComputeFullMask(x, y));

            auto it = groundAutotileMap.find(normalized);
            _levelTiles[y][x]->SetSpriteIndex(it != groundAutotileMap.end() ? it->second : 113);
            break;
        }
        case LevelObjectTileType::Slope:
        {
            uint16_t normalized = NormalizeFullMask(ComputeFullMask(x, y));

            auto itS = slopeAutotileMap.find(normalized);

            if (itS != slopeAutotileMap.end()) _levelTiles[y][x]->SetSpriteIndex(itS->second);
            else 
            {
                auto itG = groundAutotileMap.find(normalized);
                _levelTiles[y][x]->SetSpriteIndex(itG != groundAutotileMap.end() ? itG->second : 113);
            }

            uint16_t variationKey = ComputeSlopeVariationMask(normalized & 0xFF, x, y);

            auto itV = slopeVariationAutotileMap.find(variationKey);
            if (itV != slopeVariationAutotileMap.end()) {
                _levelTiles[y][x]->SetSpriteIndex(itV->second); 
            }

            break;
        }
        case LevelObjectTileType::Sand:
        case LevelObjectTileType::Dirt:
        {
            uint16_t normalized = NormalizeFullMask(ComputeFullMask(x, y));

            auto it = sandDirtAutotileMap.find(normalized);
            _levelTiles[y][x]->SetSpriteIndex(it != sandDirtAutotileMap.end() ? it->second : 48);
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

void Level::UpdateAutotileNeighbors(int x, int y, int radius)
{
    for (int dy = -radius; dy <= radius; ++dy)
    {
        for (int dx = -radius; dx <= radius; ++dx) 
        {
            int nx = x + dx; 
            int ny = y + dy;
            if (nx >= 0 && nx < kMaxLevelWidth && ny >= 0 && ny < kMaxLevelHeight)
            {
                if (_levelTiles[ny][nx]) UpdateTileAutotile(nx, ny);
            }
        }
    }
}

uint8_t Level::ComputeRawLowMask(int x, int y) const
{
    LevelObjectTileType type = _levelTiles[y][x]->GetTileType();

    uint8_t low = 0;
    if (IsTileSame(x - 1, y - 1, type)) low |= 1;       // NW
    if (IsTileSame(x, y - 1, type)) low |= 2;           // N
    if (IsTileSame(x + 1, y - 1, type)) low |= 4;       // NE
    if (IsTileSame(x - 1, y, type)) low |= 8;           // W
    if (IsTileSame(x + 1, y, type)) low |= 16;          // E
    if (IsTileSame(x - 1, y + 1, type)) low |= 32;      // SW
    if (IsTileSame(x, y + 1, type)) low |= 64;          // S
    if (IsTileSame(x + 1, y + 1, type)) low |= 128;     // SE

    return low;
}

uint8_t Level::ComputeRawHighMask(int x, int y) const
{
    uint8_t high = 0;
    if (IsTileSlope(x-1,y-1)) high |= 1;       // NW
    if (IsTileSlope(x,y-1)) high |= 2;         // N
    if (IsTileSlope(x+1,y-1)) high |= 4;       // NE
    if (IsTileSlope(x-1,y)) high |= 8;         // W
    if (IsTileSlope(x+1,y)) high |= 16;        // E
    if (IsTileSlope(x-1,y+1)) high |= 32;      // SW
    if (IsTileSlope(x,y+1)) high |= 64;        // S
    if (IsTileSlope(x+1,y+1)) high |= 128;     // SE

    return high;
}

uint16_t Level::ComputeFullMask(int x, int y) const
{
    uint8_t low  = ComputeRawLowMask(x, y);
    uint8_t high = ComputeRawHighMask(x, y);

    return (uint16_t)(low | (high << 8));
}

uint8_t Level::NormalizeLowMask(uint8_t low) const
{
    if (!(low & 2) || !(low & 8)) low &= ~1;     // NW needs N & W
    if (!(low & 2) || !(low & 16)) low &= ~4;    // NE needs N & E
    if (!(low & 64) || !(low & 8)) low &= ~32;   // SW needs S & W
    if (!(low & 64) || !(low & 16)) low &= ~128; // SE needs S & E
    return low;
}

uint16_t Level::NormalizeFullMask(uint16_t mask) const
{
    uint8_t low  = NormalizeLowMask(mask & 0xFF);
    uint8_t high = (mask >> 8) & 0xFF;

    high &= 0b01011010;
    for (int b = 0; b < 8; ++b)
        if (!(low & (1 << b))) high &= ~(1 << b);

    high = CollapseHighByte(low, high);

    return (uint16_t)(low | (high << 8));
}

uint8_t Level::CollapseHighByte(uint8_t low, uint8_t high) const
{
    switch (low)
    {
        // "sota o dreta": SE notch → col·lapsa E→S, conserva només S
        case 80: case 86: case 120: case 122: case 127:
            if (high & 16) { high |= 64; high &= ~16; }
            high &= 64;
            break;

        // "sota o esquerra": SW notch → col·lapsa W→S, conserva només S
        case 72: case 75: case 216: case 218: case 223:
            if (high & 8) { high |= 64; high &= ~8; }
            high &= 64;
            break;

        // "sobre | sota o dreta": conserva N i S (col·lapsa E→S)
        case 82: case 123: case 126:   // ← 123 era al grup equivocat!
            if (high & 16) { high |= 64; high &= ~16; }
            high &= (2 | 64);
            break;

        // "sobre | sota o esquerra": conserva N i S (col·lapsa W→S)
        case 74: case 219: case 222:
            if (high & 8) { high |= 64; high &= ~8; }
            high &= (2 | 64);
            break;

        // "dreta | esquerra": independents, conserva W i E
        case 88: case 90: case 95:
            high &= (8 | 16);
            break;

        // "sobre | dreta | esquerra": conserva N, W i E
        case 91: case 94:
            high &= (2 | 8 | 16);
            break;

        // "slope a sobre": conserva només N
        case 10: case 18: case 27: case 30:
        case 106: case 210: case 251: case 254:
            high &= 2;
            break;

        // Cap variant de slope per a la resta
        default:
            high = 0;
            break;
    }
    return high;
}

uint16_t Level::ComputeSlopeVariationMask(uint8_t maskLow, int x, int y) const
{
    uint8_t var = 0;

    switch (maskLow)
    {
        // S+E+SE — la cantonada desconeguda és la SW (la W no forma part del mask)
        case 208:
            if (!IsTileSame(x - 1, y + 1, LevelObjectTileType::Ground)) var |= 1; // SW buida
            break;

        // S+W+SW — la cantonada desconeguda és la SE (la E no forma part del mask)
        case 104:
            if (!IsTileSame(x + 1, y + 1, LevelObjectTileType::Ground)) var |= 1; // SE buida
            break;

        // S+E — mateixa cantonada lliure que el 208 (SW), més si S o E són ells mateixos un slope
        case 80:
            if (IsTileSlope(x, y + 1) || IsTileSlope(x + 1, y)) var |= 1; // S o E és slope
            if (!IsTileSame(x - 1, y + 1, LevelObjectTileType::Ground)) var |= 2; // SW buida
            break;

        // S+W — mateixa cantonada lliure que el 104 (SE), més si S o W són ells mateixos un slope
        case 72:
            if (IsTileSlope(x, y + 1) || IsTileSlope(x - 1, y)) var |= 1; // S o W és slope
            if (!IsTileSame(x + 1, y + 1, LevelObjectTileType::Ground)) var |= 2; // SE buida
            break;

        case 18:
        case 10:
            if (IsTileSlope(x, y - 1)) var |= 1; // N és slope
            break;

        default:
            return 0xFFFF; // no és cap de les 6 formes ESPECIALS
    }

    return ((uint16_t)maskLow << 8) | var;
}

uint8_t Level::ComputeTileMaskBridge(int x, int y) const
{
    auto same = [&](int cx, int cy) -> bool {
        if (cx < 0 || cx >= kMaxLevelWidth || cy < 0 || cy >= kMaxLevelHeight || !_levelTiles[cy][cx])
            return false;
        return _levelTiles[cy][cx]->GetTileType() == LevelObjectTileType::Bridge;
        };

    uint8_t mask = 0;
    if (same(x - 1, y)) mask |= 1;          // W
    if (same(x + 1, y)) mask |= 2;          // E

    return mask;
}

void Level::SetTileSideCollisionMask(int x, int y)
{
    LevelObjectTile* tile = _levelTiles[y][x].get();
    if (!tile) return;

    const LevelObjectTileType type = tile->GetTileType();
    if (type != LevelObjectTileType::Ground && type != LevelObjectTileType::Slope)
    {
        tile->SetSolidSideCollisionMask(SOLID_SIDE_LEFT, true);
        tile->SetSolidSideCollisionMask(SOLID_SIDE_TOP, true);
        tile->SetSolidSideCollisionMask(SOLID_SIDE_RIGHT, true);
        tile->SetSolidSideCollisionMask(SOLID_SIDE_BOTTOM, true);
        return;
    }

    bool hasN = false, hasS = false, hasW = false, hasE = false;

    if (x > 0)
        hasW = _levelTiles[y][x - 1] && (_levelTiles[y][x - 1]->GetTileType() == LevelObjectTileType::Ground || _levelTiles[y][x - 1]->GetTileType() == LevelObjectTileType::Slope || _levelTiles[y][x - 1]->GetTileType() == LevelObjectTileType::Dirt);
    if (x < kMaxLevelWidth - 1)
        hasE = _levelTiles[y][x + 1] && (_levelTiles[y][x + 1]->GetTileType() == LevelObjectTileType::Ground || _levelTiles[y][x + 1]->GetTileType() == LevelObjectTileType::Slope || _levelTiles[y][x + 1]->GetTileType() == LevelObjectTileType::Dirt);
    if (y > 0)
    {
        hasN = _levelTiles[y - 1][x] && (_levelTiles[y - 1][x]->GetTileType() == LevelObjectTileType::Ground || _levelTiles[y - 1][x]->GetTileType() == LevelObjectTileType::Slope || _levelTiles[y - 1][x]->GetTileType() == LevelObjectTileType::Dirt);
        if (_levelTiles[y - 1][x] && IsTileSlope(x, y - 1))
        {
            // Disable internal top collision unless the slope above it ends
			// This fixes clipping into ending slopes whilst not interfering with slope sliding behavior
            SlopeOrientation orientation = _levelTiles[y - 1][x]->GetSlopeOrientation();
            if (orientation == SlopeOrientation::RisingRight && !hasW) hasN = false;
            else if (orientation == SlopeOrientation::RisingLeft && !hasE) hasN = false;
        }
    }
    if (y < kMaxLevelHeight - 1)
    {
        hasS = _levelTiles[y + 1][x] && (_levelTiles[y + 1][x]->GetTileType() == LevelObjectTileType::Ground || _levelTiles[y + 1][x]->GetTileType() == LevelObjectTileType::Slope || _levelTiles[y + 1][x]->GetTileType() == LevelObjectTileType::Dirt);
        if (_levelTiles[y + 1][x] && IsTileSlope(x, y + 1))
        {
            // Disable internal bottom collision unless the slope below it ends
			// This fixes clipping into ending slopes whilst not interfering with slope ceiling collision behavior
			SlopeOrientation orientation = _levelTiles[y + 1][x]->GetSlopeOrientation();
            if (orientation == SlopeOrientation::FallingRight && !hasW) hasS = false;
            else if (orientation == SlopeOrientation::FallingLeft && !hasE) hasS = false;
        }
    }

    tile->SetSolidSideCollisionMask(SOLID_SIDE_LEFT, !hasW);
    tile->SetSolidSideCollisionMask(SOLID_SIDE_TOP, !hasN);
    tile->SetSolidSideCollisionMask(SOLID_SIDE_RIGHT, !hasE);
    tile->SetSolidSideCollisionMask(SOLID_SIDE_BOTTOM, !hasS);
}

bool Level::IsTileSame(int x, int y, LevelObjectTileType type) const
{
    if (x < 0 || x >= kMaxLevelWidth || y < 0 || y >= kMaxLevelHeight) return false;
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
        {
            return _levelTiles[y][x]->GetTileType() == type && !_levelTiles[y][x]->GetBreakableIsBroken();  
        }
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

bool Level::IsTileSlope(int x, int y) const
{
    if (x < 0 || x >= kMaxLevelWidth || y < 0 || y >= kMaxLevelHeight) return false;
    if (!_levelTiles[y][x]) return false;
    if (_levelTiles[y][x]->GetTileType() != LevelObjectTileType::Slope) return false;

    uint8_t low = NormalizeLowMask(ComputeRawLowMask(x, y));

    if (low == 8 || low == 16) return false;
    return slopeAutotileMap.find(low) != slopeAutotileMap.end();
}

bool Level::ManagerCanApplyBreak(const LevelObject* cause) const
{
    // Only the host or local player can apply breaks
    if (_levelManager) return _levelManager->CanApplyBreak(cause);

    return false;
}

void Level::ManagerNotifyScore(const LevelObject* cause, int points)
{
    if (!_levelManager) return;

    _levelManager->NotifyScore(cause, points);
}