#ifndef NETWORKLEVELOBJECTDYNENTSTATE_HH
#define NETWORKLEVELOBJECTDYNENTSTATE_HH

#include <cstdint>
#include "Network/Protocol/ByteStream.hh"

// State for dynamic or entities (dog, ball, etc.)
struct NetworkLevelObjectDynEntState 
{
    float x = 0, y = 0, vx = 0, vy = 0;
    uint8_t mode = 0;
    uint8_t flags = 0;
    uint8_t aux = 0;
    uint8_t interactionCause = 0xFF;
    uint8_t interactionDepth = 0;
    float t0 = 0, t1 = 0, t2 = 0, t3 = 0;
};

inline void WriteBody(ByteWriter& w, const NetworkLevelObjectDynEntState& s) 
{
    w.F32(s.x); w.F32(s.y); w.F32(s.vx); w.F32(s.vy);
    w.U8(s.mode); w.U8(s.flags); w.U8(s.aux); w.U8(s.interactionCause); w.U8(s.interactionDepth);
    w.F32(s.t0); w.F32(s.t1); w.F32(s.t2); w.F32(s.t3);
}

inline void ReadBody(ByteReader& r, NetworkLevelObjectDynEntState& s) 
{
    s.x = r.F32(); s.y = r.F32(); s.vx = r.F32(); s.vy = r.F32();
    s.mode = r.U8(); s.flags = r.U8(); s.aux = r.U8(); s.interactionCause = r.U8(); s.interactionDepth = r.U8();
    s.t0 = r.F32(); s.t1 = r.F32(); s.t2 = r.F32(); s.t3 = r.F32();
}

#endif