#include "LevelManager.hh"

LevelManager::LevelManager() :
    _level(this)
{

}

LevelManager::~LevelManager()
{

}

void LevelManager::Update(const float deltaTime)
{
    _level.Update(deltaTime);

    int w = GetScreenWidth();
    int h = GetScreenHeight();

    if (w != _screenWidth || h != _screenHeight)
    {
        _screenWidth = w;
        _screenHeight = h;

        float zoomX = (float)_screenWidth / _levelWidth;
        float zoomY = (float)_screenHeight / _levelHeight;

        _cam.zoom = std::min(zoomX, zoomY);
        _cam.offset = { _screenWidth / 2.0f, _screenHeight / 2.0f };
    }
}

void LevelManager::Render(const float deltaTime, const std::function<void()>& overlay) const
{
    BeginDrawing();
    BeginMode2D(_cam);
    ClearBackground({ 0, 0, 0, 255 });

    _level.RenderBackground(deltaTime);
    _level.RenderObjects(deltaTime);

    if (DEBUG_DRAW_BOUNDS) _level.RenderBounds(deltaTime);

    DrawFPS(-500, -500);

    EndMode2D();
    if (overlay) overlay();
    EndDrawing();
}

void LevelManager::ReceiveInput(const std::array<uint8_t, MAX_PLAYERS>& input)
{
    for (int slot = 0; slot < MAX_PLAYERS; slot++)
    {
        if (_dogBySlot[slot]) _dogBySlot[slot]->ReceiveInput(input[slot]);
    }
}

void LevelManager::LoadLevel(const std::string& levelName, const LevelPlayers& players, uint32_t spawnSeed)
{
    _dogBySlot.fill(nullptr);

    _level.ReadLevelFromFile(levelName);

    int housesNum = _level.GetDogHouseCount();
    if (housesNum <= 0) return;

    std::vector<uint8_t> houseIndexes(housesNum);
    for (int i = 0; i < housesNum; ++i) houseIndexes[i] = static_cast<uint8_t>(i);

    // Randomly shuffle the dog houses to ensure random placement of dogs, even if there are more dogs than houses
    DeterministicRng rng(spawnSeed);
    DeterministicShuffle(houseIndexes, rng);

    int spawnIndex = 0;
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        if (!players[i].active) continue;

        uint8_t houseIndex = houseIndexes[spawnIndex % housesNum];
        LevelObjectDog* dog = _level.SpawnDogInHouse(houseIndex, static_cast<uint8_t>(i), players[i].dogType);
        _dogBySlot[i] = dog;

        spawnIndex++;
    }

    _levelWidth = _level.GetLevelWidth();
    _levelHeight = _level.GetLevelHeight();
    _screenWidth = GetScreenWidth();
    _screenHeight = GetScreenHeight();

    float zoomX = (float)_screenWidth / _levelWidth;
    float zoomY = (float)_screenHeight / _levelHeight;

    _cam.zoom = std::min(zoomX, zoomY);
    _cam.target = { _levelWidth / 2.0f, _levelHeight / 2.0f };
    _cam.offset = { _screenWidth / 2.0f, _screenHeight / 2.0f };
    _cam.rotation = 0.0f;
}

void LevelManager::SetNetAuthority(bool isClient, const std::vector<uint8_t>& mySlots)
{
    _isNetClient = isClient;
    _localSlotMask = 0;
    for (uint8_t s : mySlots) if (s < MAX_PLAYERS) _localSlotMask |= (1u << s);
}

bool LevelManager::CanApplyBreak(const LevelObject* cause) const
{
    if (!_isNetClient) return true;
    if (!cause) return false;
    const uint8_t slot = cause->GetInteractionCauseSlot();
    return slot < MAX_PLAYERS && (_localSlotMask & (1u << slot)) != 0;
}

void LevelManager::NotifyScore(const LevelObject* cause, int basePoints)
{
    if (!_onScore || !cause) return;
    const uint8_t slot = cause->GetInteractionCauseSlot();
    if (slot < MAX_PLAYERS) _onScore(slot, basePoints + cause->GetInteractionDepth());
}

void LevelManager::SetSlotGone(uint8_t slot)
{
    if (slot < MAX_PLAYERS && _dogBySlot[slot]) _dogBySlot[slot]->SetGone();
}