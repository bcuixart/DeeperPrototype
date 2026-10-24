#ifndef LOBBYEVENTS_HH
#define LOBBYEVENTS_HH

#include <vector>
#include "Network/NetworkTypes.hh"
#include "Network/Protocol/ByteStream.hh"

// Shared with RoundEvent (1..3) and MatchEvent (4..5): numbers cannot be repeated
enum class LobbyEvent : uint8_t { SetDogType = 6 };

enum class LobbyOption : uint8_t { RoundCount, RoundSeconds };

inline std::vector<uint8_t> EncodeSetDogType(uint8_t slot, uint8_t dogType)
{
    ByteWriter w;
    w.U8(slot);
    w.U8(dogType);
    return w.Data();
}

inline bool DecodeSetDogType(ByteReader& r, uint8_t& slot, uint8_t& dogType)
{
    slot = r.U8();
    dogType = r.U8();
    return r.Ok() && slot < kMaxSlots && dogType < kNumDogTypes;
}

#endif