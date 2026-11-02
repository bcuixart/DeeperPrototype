#ifndef LEVELMANAGER_HH
#define LEVELMANAGER_HH

#include <vector>
#include <array>
#include <memory>
#include <limits>
#include <functional>

#include "Level.hh"
#include "Utils/DeterministicRNG.hh"
#include "LevelObjects/LevelObjectDog.hh"

struct LevelPlayerSlot { bool active = false; uint8_t dogType = 0; };
using LevelPlayers = std::array<LevelPlayerSlot, MAX_PLAYERS>;

class LevelManager {
public:
	LevelManager();
	~LevelManager();

	void Update(const float deltaTime);
	void Render(const float deltaTime, const std::function<void()>& overlay = nullptr) const;

	void ReceiveInput(const std::array<uint8_t, MAX_PLAYERS>& input);

	void LoadLevelFromFile(const std::string& fileName, const LevelPlayers& players, uint32_t spawnSeed);
	void LoadLevelFromStream(std::istream& stream, const LevelPlayers& players, uint32_t spawnSeed);

	void SetScoreCallback(std::function<void(uint8_t, int)> cb) { _onScore = std::move(cb); }
	void NotifyScore(const LevelObject* cause, int points);

    void SetNetAuthority(bool isClient, const std::vector<uint8_t>& mySlots);
    bool CanApplyBreak(const LevelObject* cause) const;

	std::vector<LevelObject*> GetNetBodies() const { return _level.GetNetBodies(); }
	const std::vector<LevelObject*>& GetNetBreakableTiles() const { return _level.GetNetBreakableTiles(); }

	void SetSlotGone(uint8_t slot);

protected:

private:
	void SpawnDogs(const LevelPlayers& players, uint32_t spawnSeed);

	Level _level;

	Camera2D _cam = { 0 };

	std::function<void(uint8_t, int)> _onScore;

	std::array<LevelObjectDog*, MAX_PLAYERS> _dogBySlot{};

    bool _isNetClient = false;
    uint8_t _localSlotMask = 0;

	constexpr static bool DEBUG_DRAW_BOUNDS = false;
};

#endif