#include "LevelNetworkSync.hh"
#include "LevelObject.hh"
#include <algorithm>
#include <cmath>
#include <cstdio>

void LevelNetworkSync::Reset(const std::vector<LevelObject*>& bodies, const std::vector<LevelObject*>& sandTiles) {
    _bodies = bodies;
    _sand = sandTiles;
    for (Frame& f : _history) { f.valid = false; f.bodies.clear(); f.sand.clear(); }
    _maxError = 0; _corrections = 0;
    _lastSand = PackSand();
    _sandChanged = false;
}

std::vector<uint8_t> LevelNetworkSync::PackSand() const {
    std::vector<uint8_t> bits((_sand.size() + 7) / 8, 0);
    for (size_t i = 0; i < _sand.size(); ++i)
        if (_sand[i]->GetBreakableIsBroken()) bits[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
    return bits;
}

void LevelNetworkSync::ApplySand(const std::vector<uint8_t>& bits) {
    for (size_t i = 0; i < _sand.size(); ++i)
        _sand[i]->NetworkSetBroken(((bits[i / 8] >> (i % 8)) & 1) != 0);
}

int LevelNetworkSync::CountBroken() const {
    int n = 0;
    for (const LevelObject* o : _sand) if (o->GetBreakableIsBroken()) ++n;
    return n;
}

void LevelNetworkSync::Record(uint32_t tick) {
    Frame& f = _history[tick % kHistory];
    f.valid = true;
    f.tick = tick;
    f.bodies.assign(_bodies.size(), NetworkLevelObjectDynEntState{});
    for (size_t i = 0; i < _bodies.size(); ++i) _bodies[i]->NetGet(f.bodies[i]);
    f.sand = PackSand();
    _sandChanged = (f.sand != _lastSand);
    _lastSand = f.sand;
}

bool LevelNetworkSync::RestoreTo(uint32_t tick) 
{
    const Frame& f = _history[tick % kHistory];
    if (!f.valid || f.tick != tick || f.bodies.size() != _bodies.size()) return false;
    for (size_t i = 0; i < _bodies.size(); ++i) _bodies[i]->NetSet(f.bodies[i]);
    ApplySand(f.sand);
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
    w.U16(static_cast<uint16_t>(_sand.size()));
    for (uint8_t b : PackSand()) w.U8(b);
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

    const size_t nSand = r.U16();
    if (!r.Ok() || nSand != _sand.size() || mine.sand.size() != (nSand + 7) / 8) return;
    std::vector<uint8_t> hostSand((nSand + 7) / 8);
    for (auto& b : hostSand) b = r.U8();
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
    const bool sandMismatch = (hostSand != mine.sand);
    _maxError = std::max(_maxError, worst);
    if (!mismatch && !sandMismatch) return;
    ++_corrections;

    if (worst > 1.0f || sandMismatch)
        printf("[rollback] T=%u L=%u error maxim %.2f%s\n", hostTick, latestTick, worst,
               sandMismatch ? " | sorra diferent" : "");

    // L'estat del host al tick T és la veritat: l'adoptem i tornem a simular fins ara.
    mine.bodies = host;
    mine.sand = hostSand;
    for (size_t i = 0; i < n; ++i) _bodies[i]->NetSet(host[i]);
    ApplySand(hostSand);

    for (uint32_t t = hostTick + 1; t <= latestTick; ++t) {
        replayTick(t);
        Record(t);
    }
}