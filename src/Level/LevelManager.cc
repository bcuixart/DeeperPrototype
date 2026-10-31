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
}

void LevelManager::Render(const float deltaTime, const std::function<void()>& overlay) const
{
    BeginDrawing();
    BeginMode2D(_cam);
    ClearBackground({ 0, 196, 255, 255 });

    _level.Render(deltaTime);

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

    _level.LoadLevel(levelName);

    int housesNum = _level.GetDogHouseCount();
    if (housesNum <= 0) return;

    std::vector<uint8_t> houseIndexes(housesNum);
    for (int i = 0; i < housesNum; ++i) houseIndexes[i] = static_cast<uint8_t>(i);

    // Randomly shuffle the dog houses to ensure random placement of dogs, even if there are more dogs than houses
    DeterministicRng rng(spawnSeed);
    DeterministicShuffle(houseIndexes, rng);

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        if (!players[i].active) continue;

        uint8_t houseIndex = houseIndexes[i] % housesNum;
        LevelObjectDog* dog = _level.SpawnDogInHouse(houseIndex, static_cast<uint8_t>(i), players[i].dogType);
        _dogBySlot[i] = dog;
    }

    float zoomX = (float)GetScreenWidth() / _level.GetLevelWidth();
    float zoomY = (float)GetScreenHeight() / _level.GetLevelHeight();

    _cam.zoom = std::min(zoomX, zoomY);
    _cam.target = { _level.GetLevelWidth() / 2.0f, _level.GetLevelHeight() / 2.0f };
    _cam.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
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