#ifndef NETWORK_TYPES_HH
#define NETWORK_TYPES_HH

#include <array>
#include <cstdint>
#include <vector>
#include <string>

constexpr int      kMaxSlots         = 8;
constexpr int      kMaxPlayersPerMachine = 4;
constexpr uint16_t kProtocolVersion  = 12;
constexpr uint8_t  kNoMachine        = 0xFF;
constexpr int      kInputRedundancy  = 6;
constexpr uint32_t kSnapshotInterval = 6;
constexpr size_t   kMaxSnapshotBytes = 1000;
constexpr size_t   kMaxEventBytes    = 255;
constexpr uint8_t  kNumDogTypes      = 5;
constexpr uint8_t  kNumCursorPoses   = 16;
constexpr float    kCursorScale      = 16.0f;

// Lobby options
constexpr uint8_t  kMinRounds        = 1;
constexpr uint8_t  kMaxRounds        = 20;
constexpr uint16_t kMinRoundSeconds  = 10;
constexpr uint16_t kMaxRoundSeconds  = 180;

constexpr bool kAllowLobbyDogChange  = false;

// A machine represents a single instance of the game running on a computer. Each machine can have multiple players connected to it.
// The host will always be machine 0.
struct SlotInfo 
{
    uint8_t machineId = kNoMachine;
    uint8_t dogType   = 0;

    bool Occupied() const { return machineId != kNoMachine; }
    bool operator==(const SlotInfo& o) const { return machineId == o.machineId && dogType == o.dogType; }
};

struct LobbySettings
{
    uint8_t  roundCount   = 3;
    uint16_t roundSeconds = 20;

    bool IsValid() const
    {
        return roundCount >= kMinRounds && roundCount <= kMaxRounds
            && roundSeconds >= kMinRoundSeconds && roundSeconds <= kMaxRoundSeconds;
    }
    bool operator==(const LobbySettings& o) const { return roundCount == o.roundCount && roundSeconds == o.roundSeconds; }
};

struct LobbyState 
{
    std::array<SlotInfo, kMaxSlots> slots;
    LobbySettings settings;

    bool operator==(const LobbyState& o) const { return slots == o.slots && settings == o.settings; }
    bool operator!=(const LobbyState& o) const { return !(*this == o); }
};

// The info needed to start a level. This is sent from the host to all clients when the game starts.
struct LevelStartInfo 
{
    LobbyState  lobby;
    uint32_t    spawnSeed = 0;  // Random seed for assigning houses to dogs.
    uint32_t    roundTicks = 1200;
    uint32_t    levelHash = 0;
    uint8_t     roundIndex = 0;
    uint8_t     roundCount = 1; 
    std::string levelName;
};

struct GameEventMsg 
{
    uint8_t kind = 0;
    uint8_t fromMachine = kNoMachine;
    std::vector<uint8_t> payload;
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

struct CursorState
{
    uint8_t  slot = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint8_t  pose = 0;
};

struct CursorPacket
{
    std::vector<CursorState> cursors;
};

#endif