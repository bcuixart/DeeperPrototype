#ifndef NETWORK_TYPES_HH
#define NETWORK_TYPES_HH

#include <array>
#include <cstdint>

constexpr int      kMaxSlots        = 8;   // TODO: Make it MAX_PLAYERS
constexpr int      kMaxPlayersPerMachine = 4; // TODO: Make it MAX_LOCAL_PLAYERS
constexpr uint16_t kProtocolVersion = 1;
constexpr uint8_t  kNoMachine       = 0xFF;

// Una "màquina" és un ordinador connectat. La 0 és sempre el host.
struct SlotInfo 
{
    uint8_t machineId = kNoMachine;

    bool Occupied() const { return machineId != kNoMachine; }
    bool operator==(const SlotInfo& o) const { return machineId == o.machineId; }
};

struct LobbyState 
{
    std::array<SlotInfo, kMaxSlots> slots;

    bool operator==(const LobbyState& o) const { return slots == o.slots; }
    bool operator!=(const LobbyState& o) const { return !(*this == o); }
};

#endif