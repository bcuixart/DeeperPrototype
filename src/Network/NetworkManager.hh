#ifndef NETWORK_MANAGER_HH
#define NETWORK_MANAGER_HH

#include "NetworkTypes.hh"
#include "Protocol/NetworkProtocol.hh"
#include "Transport/ITransport.hh"
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <deque>

class NetworkManager {
public:
    enum class State { Idle, Connecting, InLobby, InGame, Failed };

    explicit NetworkManager(std::unique_ptr<ITransport> transport);

    // dogTypes: un element per jugador local (1 a kMaxPlayersPerMachine)
    bool Host(uint16_t port, const std::vector<uint8_t>& dogTypes);
    void Join(const std::string& address, uint16_t port, const std::vector<uint8_t>& dogTypes);
    void Leave();
    void Update();   // cridar un cop per frame

    // Només host, només al lobby. Tanca el lobby i avisa a tothom.
    bool HostStartLevel(const std::string& levelName, uint32_t roundTicks);
    // Retorna true un cop quan s'ha de començar el nivell (també al host).
    bool PopLevelStart(LevelStartInfo& out);

    void SendLocalInputs(uint32_t tick, const std::vector<uint8_t>& mySlots, const std::array<uint8_t, kMaxSlots>& slotInput);
    bool PopReceivedInput(InputPacket& out);
    int GetRttMs() const;
    
    void SetLocalTick(uint32_t tick) { _localTick = tick; }
    void SendTimePing();
    bool PopTimePong(uint32_t& hostTick, int& rttMs);

    void HostSendSnapshot(uint32_t tick, const std::vector<uint8_t>& blob);
    bool PopSnapshot(uint32_t& tick, std::vector<uint8_t>& blob);

    void HostBroadcastEvent(uint8_t kind, const std::vector<uint8_t>& payload);
    void SendEventToHost(uint8_t kind, const std::vector<uint8_t>& payload);
    bool PopEvent(GameEventMsg& out);

    bool PopLeftSlot(uint8_t& slot);

    State GetState() const { return _state; }
    bool IsHost() const { return _isHost; }
    uint8_t GetMyMachineId() const { return _myMachineId; }
    const LobbyState& GetLobby() const { return _lobby; }
    const std::string& GetFailReason() const { return _failReason; }
    // Slots que són meus, en ordre ascendent: el jugador local i correspon a [i].
    std::vector<uint8_t> GetMySlots() const;

private:
    using Clock = std::chrono::steady_clock;
    static constexpr int kConnectTimeoutMs = 5000;

    void HandleEvent(const NetEvent& ev);
    void HostHandleMessage(PeerId peer, ByteReader& r);
    void ClientHandleMessage(ByteReader& r);

    bool AllocSlots(uint8_t machineId, const std::vector<uint8_t>& dogTypes);
    void FreeSlots(uint8_t machineId);
    uint8_t NextFreeMachineId() const;
    void BroadcastLobby();
    void SendTo(PeerId peer, Channel ch, const std::vector<uint8_t>& msg);
    void Fail(const std::string& reason);

    void SendToAll(Channel ch, const std::vector<uint8_t>& msg, PeerId except = kInvalidPeer);
    void HostHandleInput(PeerId peer, ByteReader& r);
    void PushReceivedInput(InputPacket&& packet);

    void PushEvent(GameEventMsg&& ev);
    std::deque<GameEventMsg> _events;

    struct InputHistory 
    {
        std::array<uint8_t, kInputRedundancy> inputs{};
        uint8_t count = 0;
    };
    std::array<InputHistory, kMaxSlots> _inputHistory;
    std::deque<InputPacket> _receivedInputs;

    std::unique_ptr<ITransport> _transport;
    State _state = State::Idle;
    bool _isHost = false;
    uint8_t _myMachineId = kNoMachine;
    LobbyState _lobby;
    std::string _failReason;
    std::optional<LevelStartInfo> _pendingLevelStart;

    bool _hasSnapshot = false;
    uint32_t _snapTick = 0;
    std::vector<uint8_t> _snapBlob;

    struct TimePong { uint32_t hostTick; int rttMs; };
    std::optional<TimePong> _timePong;
    uint32_t _localTick = 0;
    Clock::time_point _epoch = Clock::now();
    uint32_t NowMs() const;

    static constexpr int kSilenceTimeoutMs = 4000;
    std::unordered_map<uint8_t, Clock::time_point> _lastHeard;
    std::deque<uint8_t> _leftSlots;
    void HostDropMachine(uint8_t machineId);

    // Només host: quina màquina és cada peer.
    std::map<PeerId, uint8_t> _machineOfPeer;

    // Només client:
    PeerId _serverPeer = 0;
    std::vector<uint8_t> _localDogTypes;
    Clock::time_point _connectStart;
};

#endif