#include "LevelNetworkSync.hh"
#include "LevelObject.hh"
#include <algorithm>
#include <cmath>

void LevelNetworkSync::Reset(const std::vector<LevelObject*>& bodies) {
    _bodies = bodies;
    for (Frame& f : _history) { f.valid = false; f.bodies.clear(); }
    _maxError = 0; _corrections = 0;
}

void LevelNetworkSync::Record(uint32_t tick) {
    Frame& f = _history[tick % kHistory];
    f.valid = true;
    f.tick = tick;
    f.bodies.assign(_bodies.size(), NetworkLevelObjectDynEntState{});
    for (size_t i = 0; i < _bodies.size(); ++i) _bodies[i]->NetGet(f.bodies[i]);
}

std::vector<uint8_t> LevelNetworkSync::BuildSnapshot() const {
    ByteWriter w;
    w.U8(static_cast<uint8_t>(_bodies.size()));
    for (const LevelObject* o : _bodies) {
        NetworkLevelObjectDynEntState s;
        o->NetGet(s);
        WriteBody(w, s);
    }
    return w.Data();
}

void LevelNetworkSync::ApplySnapshot(uint32_t hostTick, const std::vector<uint8_t>& blob,
    uint32_t latestTick, const ReplayFn& replayTick) {
    if (latestTick < hostTick || latestTick - hostTick >= kHistory) return;

    Frame& mine = _history[hostTick % kHistory];
    if (!mine.valid || mine.tick != hostTick) return;

    ByteReader r(blob.data(), blob.size());
    const size_t n = r.U8();
    if (!r.Ok() || n != _bodies.size() || mine.bodies.size() != n) return;

    std::vector<NetworkLevelObjectDynEntState> host(n);
    for (auto& h : host) ReadBody(r, h);
    if (!r.Ok()) return;

    // Ens val la pena resimular? Només si algun cos difereix del que va dir el host.
    bool mismatch = false;
    float worst = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const auto& h = host[i];
        const auto& m = mine.bodies[i];
        const float err = std::hypot(h.x - m.x, h.y - m.y);
        const float verr = std::hypot(h.vx - m.vx, h.vy - m.vy);
        worst = std::max(worst, err);
        if (err > kIgnoreError || verr > kIgnoreVelocity || h.mode != m.mode || h.flags != m.flags)
            mismatch = true;
    }
    _maxError = std::max(_maxError, worst);
    if (!mismatch) return;
    ++_corrections;

    if (worst > 1.0f)
        printf("[rollback] T=%u L=%u (%u ticks) error maxim %.2f\n", hostTick, latestTick, latestTick - hostTick, worst);

    // L'estat del host al tick T és la veritat: l'adoptem i tornem a simular fins ara.
    mine.bodies = host;
    for (size_t i = 0; i < n; ++i) _bodies[i]->NetSet(host[i]);

    for (uint32_t t = hostTick + 1; t <= latestTick; ++t) {
        replayTick(t);
        Record(t);
    }
}