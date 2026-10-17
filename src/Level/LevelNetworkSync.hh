#ifndef LEVELNETWORKSYNC_HH
#define LEVELNETWORKSYNC_HH

#include <array>
#include <cstdint>
#include <functional>
#include <vector>
#include "NetworkLevelObjectDynEntState.hh"

class LevelObject;

class LevelNetworkSync {
public:
    using ReplayFn = std::function<void(uint32_t tick)>;

    void Reset(const std::vector<LevelObject*>& bodies, const std::vector<LevelObject*>& sandTiles);

    // Crida-ho DESPRÉS de simular el tick (host i client).
    void Record(uint32_t tick);
    bool RestoreTo(uint32_t tick);

    // Host: ha canviat alguna tile de sorra en l'últim Record? (per enviar un snapshot extra)
    bool SandChanged() const { return _sandChanged; }

    std::vector<uint8_t> BuildSnapshot() const;

    void ApplySnapshot(uint32_t hostTick, const std::vector<uint8_t>& blob,
                       uint32_t latestTick, const ReplayFn& replayTick);

    float TakeMaxError()    { float e = _maxError; _maxError = 0; return e; }
    int   TakeCorrections() { int c = _corrections; _corrections = 0; return c; }
    int   CountBroken() const;   // depuració

private:
    static constexpr uint32_t kHistory = 64;
    static constexpr float kIgnoreError    = 0.02f;
    static constexpr float kIgnoreVelocity = 0.1f;

    struct Frame {
        bool valid = false;
        uint32_t tick = 0;
        std::vector<NetworkLevelObjectDynEntState> bodies;
        std::vector<uint8_t> sand;      // bitset de sorra trencada
    };

    std::vector<uint8_t> PackSand() const;
    void ApplySand(const std::vector<uint8_t>& bits);

    std::vector<LevelObject*> _bodies;
    std::vector<LevelObject*> _sand;
    std::array<Frame, kHistory> _history;
    std::vector<uint8_t> _lastSand;
    bool _sandChanged = false;
    float _maxError = 0;
    int _corrections = 0;
};

#endif