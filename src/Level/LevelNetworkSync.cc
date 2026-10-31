#include "LevelNetworkSync.hh"
#include "LevelObject.hh"
#include <algorithm>
#include <cmath>
#include <cstdio>

void LevelNetworkSync::Reset(const std::vector<LevelObject*>& bodies, const std::vector<LevelObject*>& breakableTiles) {
    _bodies = bodies;
    _breakableTiles = breakableTiles;
    for (Frame& f : _history) { f.valid = false; f.bodies.clear(); f.breakableTiles.clear(); }
    _maxError = 0; _corrections = 0;
    _lastBreakableTiles = PackBreakableTiles();
    _breakableTilesChanged = false;
}

std::vector<uint8_t> LevelNetworkSync::PackBreakableTiles() const {
    std::vector<uint8_t> bits((_breakableTiles.size() + 7) / 8, 0);
    for (size_t i = 0; i < _breakableTiles.size(); ++i)
        if (_breakableTiles[i]->GetBreakableIsBroken()) bits[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
    return bits;
}

void LevelNetworkSync::ApplyBreakableTiles(const std::vector<uint8_t>& bits) {
    for (size_t i = 0; i < _breakableTiles.size(); ++i)
        _breakableTiles[i]->NetworkSetBroken(((bits[i / 8] >> (i % 8)) & 1) != 0);
}

int LevelNetworkSync::CountBroken() const {
    int n = 0;
    for (const LevelObject* o : _breakableTiles) if (o->GetBreakableIsBroken()) ++n;
    return n;
}

void LevelNetworkSync::Record(uint32_t tick) {
    Frame& f = _history[tick % kHistory];
    f.valid = true;
    f.tick = tick;
    f.bodies.assign(_bodies.size(), NetworkLevelObjectDynEntState{});
    for (size_t i = 0; i < _bodies.size(); ++i) _bodies[i]->NetGet(f.bodies[i]);
    f.breakableTiles = PackBreakableTiles();
    _breakableTilesChanged = (f.breakableTiles != _lastBreakableTiles);
    _lastBreakableTiles = f.breakableTiles;
}

bool LevelNetworkSync::RestoreTo(uint32_t tick) 
{
    const Frame& f = _history[tick % kHistory];
    if (!f.valid || f.tick != tick || f.bodies.size() != _bodies.size()) return false;
    for (size_t i = 0; i < _bodies.size(); ++i) _bodies[i]->NetSet(f.bodies[i]);
    ApplyBreakableTiles(f.breakableTiles);
    return true;
}

std::vector<uint8_t> LevelNetworkSync::BuildSnapshot() const {
    ByteWriter w;
    w.U8(static_cast<uint8_t>(_bodies.size()));
    for (const LevelObject* o : _bodies) {
        NetworkLevelObjectDynEntState s;
        o->NetGet(s);
        WriteBody(w, s);
    }
    w.U16(static_cast<uint16_t>(_breakableTiles.size()));
    for (uint8_t b : PackBreakableTiles()) w.U8(b);
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

    const size_t nBreakableTiles = r.U16();
    if (!r.Ok() || nBreakableTiles != _breakableTiles.size() || mine.breakableTiles.size() != (nBreakableTiles + 7) / 8) return;
    std::vector<uint8_t> hostBreakableTiles((nBreakableTiles + 7) / 8);
    for (auto& b : hostBreakableTiles) b = r.U8();
    if (!r.Ok()) return;

    bool mismatch = false;
    float worst = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const auto& h = host[i];
        const auto& m = mine.bodies[i];
        const float err  = std::hypot(h.x - m.x, h.y - m.y);
        const float verr = std::hypot(h.vx - m.vx, h.vy - m.vy);
        worst = std::max(worst, err);
        if (err > kIgnoreError || verr > kIgnoreVelocity || h.mode != m.mode || h.flags != m.flags)
            mismatch = true;
    }
    const bool breakableTilesMismatch = (hostBreakableTiles != mine.breakableTiles);
    _maxError = std::max(_maxError, worst);
    if (!mismatch && !breakableTilesMismatch) return;
    ++_corrections;

    if (worst > 1.0f || breakableTilesMismatch)
        printf("[rollback] T=%u L=%u error maxim %.2f%s\n", hostTick, latestTick, worst,
               breakableTilesMismatch ? " | tiles trencats diferents" : "");

    // L'estat del host al tick T és la veritat: l'adoptem i tornem a simular fins ara.
    mine.bodies = host;
    mine.breakableTiles = hostBreakableTiles;
    for (size_t i = 0; i < n; ++i) _bodies[i]->NetSet(host[i]);
    ApplyBreakableTiles(hostBreakableTiles);

    for (uint32_t t = hostTick + 1; t <= latestTick; ++t) {
        replayTick(t);
        Record(t);
    }
}