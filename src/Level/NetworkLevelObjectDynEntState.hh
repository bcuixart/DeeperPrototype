#ifndef NETWORKLEVELOBJECTDYNENTSTATE_HH
#define NETWORKLEVELOBJECTDYNENTSTATE_HH

#include <cstdint>
#include "Network/Protocol/ByteStream.hh"

// State for dynamic or entities (dog, ball, etc.)
struct NetworkLevelObjectDynEntState 
{
    float x = 0, y = 0, vx = 0, vy = 0;
    uint8_t mode = 0;    // dog: DogState
    uint8_t flags = 0;   // dog: bit0 isJumping, bit1 isSlidingLeft
    float t0 = 0, t1 = 0, t2 = 0; // dog: jumpTimer, coyoteTimer, jumpInputBufferTimer
};

inline void WriteBody(ByteWriter& w, const NetworkLevelObjectDynEntState& s) 
{
    w.F32(s.x); w.F32(s.y); w.F32(s.vx); w.F32(s.vy);
    w.U8(s.mode); w.U8(s.flags);
    w.F32(s.t0); w.F32(s.t1); w.F32(s.t2);
}

inline void ReadBody(ByteReader& r, NetworkLevelObjectDynEntState& s) 
{
    s.x = r.F32(); s.y = r.F32(); s.vx = r.F32(); s.vy = r.F32();
    s.mode = r.U8(); s.flags = r.U8();
    s.t0 = r.F32(); s.t1 = r.F32(); s.t2 = r.F32();
}

#endif