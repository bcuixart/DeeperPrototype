#ifndef NETWORK_TYPES_HH
#define NETWORK_TYPES_HH

#include <array>
#include <cstdint>
#include <vector>
#include <string>

constexpr int      kMaxSlots        = 8;
constexpr int      kMaxPlayersPerMachine = 4;
constexpr uint16_t kProtocolVersion = 3;
constexpr uint8_t  kNoMachine       = 0xFF;
constexpr int      kInputRedundancy = 6;

// A machine represents a single instance of the game running on a computer. Each machine can have multiple players connected to it.
// The host will always be machine 0.
struct SlotInfo 
{
    uint8_t machineId = kNoMachine;
    uint8_t dogType   = 0;

    bool Occupied() const { return machineId != kNoMachine; }
    bool operator==(const SlotInfo& o) const { return machineId == o.machineId && dogType == o.dogType; }
};

struct LobbyState 
{
    std::array<SlotInfo, kMaxSlots> slots;

    bool operator==(const LobbyState& o) const { return slots == o.slots; }
    bool operator!=(const LobbyState& o) const { return !(*this == o); }
};

// The info needed to start a level. This is sent from the host to all clients when the game starts.
struct LevelStartInfo 
{
    LobbyState  lobby;
    uint32_t    spawnSeed = 0;  // Random seed for assigning houses to dogs.
    std::string levelName;
};

struct SlotInputs 
{
    uint8_t slot = 0;
    uint8_t count = 0;
    std::array<uint8_t, kInputRedundancy> inputs{};
};

struct InputPacket 
{
    uint32_t tick = 0;
    std::vector<SlotInputs> slots;
};

#endif