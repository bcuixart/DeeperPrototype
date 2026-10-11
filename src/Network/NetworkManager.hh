#ifndef NETWORK_MANAGER_HH
#define NETWORK_MANAGER_HH

#include "NetworkTypes.hh"
#include "Protocol/NetworkProtocol.hh"
#include "Transport/ITransport.hh"
#include <chrono>
#include <map>
#include <memory>
#include <string>

class NetworkManager {
public:
    enum class State { Idle, Connecting, InLobby, Failed };

    explicit NetworkManager(std::unique_ptr<ITransport> transport);

    bool Host(uint16_t port, int localPlayers);
    void Join(const std::string& address, uint16_t port, int localPlayers);
    void Leave();

    void Update();

    State GetState() const { return _state; }
    bool IsHost() const { return _isHost; }
    uint8_t GetMyMachineId() const { return _myMachineId; }
    const LobbyState& GetLobby() const { return _lobby; }
    const std::string& GetFailReason() const { return _failReason; }

private:
    using Clock = std::chrono::steady_clock;
    static constexpr int kConnectTimeoutMs = 5000;

    void HandleEvent(const NetEvent& ev);
    void HostHandleMessage(PeerId peer, ByteReader& r);
    void ClientHandleMessage(ByteReader& r);

    bool AllocSlots(uint8_t machineId, int count);
    void FreeSlots(uint8_t machineId);
    uint8_t NextFreeMachineId() const;
    void BroadcastLobby();
    void SendTo(PeerId peer, Channel ch, const std::vector<uint8_t>& msg);
    void Fail(const std::string& reason);

    std::unique_ptr<ITransport> _transport;
    State _state = State::Idle;
    bool _isHost = false;
    uint8_t _myMachineId = kNoMachine;
    LobbyState _lobby;
    std::string _failReason;

    std::map<PeerId, uint8_t> _machineOfPeer;

    PeerId _serverPeer = 0;
    int _requestedPlayers = 1;
    Clock::time_point _connectStart;
};

#endif