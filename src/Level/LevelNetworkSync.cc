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

void LevelNetworkSync::ApplySnapshot(uint32_t hostTick, const std::vector<uint8_t>& blob, uint32_t latestTick) {
    // El client va per davant del host, així que normalment latestTick >= hostTick.
    if (latestTick < hostTick || latestTick - hostTick >= kHistory) return;

    const Frame& mine = _history[hostTick % kHistory];
    const Frame& latest = _history[latestTick % kHistory];
    if (!mine.valid || mine.tick != hostTick) return;
    if (!latest.valid || latest.tick != latestTick) return;

    ByteReader r(blob.data(), blob.size());
    const size_t n = r.U8();
    if (!r.Ok() || n != _bodies.size() || mine.bodies.size() != n) return;

    std::vector<NetworkLevelObjectDynEntState> host(n);
    for (NetworkLevelObjectDynEntState& h : host) ReadBody(r, h);
    if (!r.Ok()) return;

    for (size_t i = 0; i < n; ++i) {
        const NetworkLevelObjectDynEntState& h = host[i];
        const NetworkLevelObjectDynEntState& m = mine.bodies[i]; // còpia: més avall modifiquem aquest frame

        const float err = std::hypot(h.x - m.x, h.y - m.y);
        _maxError = std::max(_maxError, err);

        const bool discrete = (h.mode != m.mode) || (h.flags != m.flags);
        float s;
        if (discrete || err >= kSnapError) s = 1.0f;
        else if (err < kIgnoreError)       s = 0.0f;
        else                               s = kBlend;
        if (s == 0.0f) continue;
        ++_corrections;

        if (err > 1.0f) 
        {
            const NetworkLevelObjectDynEntState& c = latest.bodies[i];
            printf("[snap] cos %zu T=%u L=%u s=%.2f | host(%.2f,%.2f v%.1f,%.1f m%d) | meu@T(%.2f,%.2f v%.1f,%.1f m%d) | meu@L(%.2f,%.2f v%.1f,%.1f)\n",
                   i, hostTick, latestTick, s,
                   h.x, h.y, h.vx, h.vy, h.mode,
                   m.x, m.y, m.vx, m.vy, m.mode,
                   c.x, c.y, c.vx, c.vy);
        }

        const float dx = (h.x - m.x) * s,   dy = (h.y - m.y) * s;
        const float dvx = (h.vx - m.vx) * s, dvy = (h.vy - m.vy) * s;
        const float d0 = (h.t0 - m.t0) * s, d1 = (h.t1 - m.t1) * s, d2 = (h.t2 - m.t2) * s;

        // Desplacem tot l'historial a partir de hostTick perquè el proper
        // snapshot no vegi (i corregeixi) un error que ja hem corregit.
        for (uint32_t t = hostTick; t <= latestTick; ++t) {
            Frame& f = _history[t % kHistory];
            if (!f.valid || f.tick != t) continue;
            NetworkLevelObjectDynEntState& b = f.bodies[i];
            b.x += dx; b.y += dy; b.vx += dvx; b.vy += dvy;
            b.t0 = std::max(0.0f, b.t0 + d0);
            b.t1 = std::max(0.0f, b.t1 + d1);
            b.t2 = std::max(0.0f, b.t2 + d2);
            if (discrete) { b.mode = h.mode; b.flags = h.flags; }
        }
        _bodies[i]->NetSet(latest.bodies[i]);
    }
}