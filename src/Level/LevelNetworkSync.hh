#ifndef LEVELNETWORKSYNC_HH
#define LEVELNETWORKSYNC_HH

#include <array>
#include <cstdint>
#include <vector>
#include "NetworkLevelObjectDynEntState.hh"

class LevelObject;

// Construeix snapshots (host) i els aplica contra l'historial propi (client).
class LevelNetworkSync {
public:
    void Reset(const std::vector<LevelObject*>& bodies);

    // Crida-ho DESPRÉS de simular el tick (host i client).
    void Record(uint32_t tick);

    // Host: estat actual dels cossos.
    std::vector<uint8_t> BuildSnapshot() const;

    // Client: hostTick = tick del snapshot; latestTick = últim tick ja simulat (_levelTick - 1).
    void ApplySnapshot(uint32_t hostTick, const std::vector<uint8_t>& blob, uint32_t latestTick);

    // Per depurar: màxim error (en tiles) i nombre de correccions des de l'última crida.
    float TakeMaxError()    { float e = _maxError; _maxError = 0; return e; }
    int   TakeCorrections() { int c = _corrections; _corrections = 0; return c; }

private:
    static constexpr uint32_t kHistory = 64;     // ~1 s a 60 Hz
    static constexpr float kIgnoreError = 0.02f; // tiles: soroll de float
    static constexpr float kSnapError   = 1.5f;  // tiles: a partir d'aquí, snap
    static constexpr float kBlend       = 0.35f; // fracció corregida per snapshot

    struct Frame {
        bool valid = false;
        uint32_t tick = 0;
        std::vector<NetworkLevelObjectDynEntState> bodies;
    };

    std::vector<LevelObject*> _bodies;
    std::array<Frame, kHistory> _history;
    float _maxError = 0;
    int _corrections = 0;
};

#endif