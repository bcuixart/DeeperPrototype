#pragma once
#include "NetworkTypes.hh"
#include "Protocol/NetworkProtocol.hh"
#include "Transport/ITransport.hh"
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
    bool HostStartLevel(const std::string& levelName);
    // Retorna true un cop quan s'ha de començar el nivell (també al host).
    bool PopLevelStart(LevelStartInfo& out);

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

    std::unique_ptr<ITransport> _transport;
    State _state = State::Idle;
    bool _isHost = false;
    uint8_t _myMachineId = kNoMachine;
    LobbyState _lobby;
    std::string _failReason;
    std::optional<LevelStartInfo> _pendingLevelStart;

    // Només host: quina màquina és cada peer.
    std::map<PeerId, uint8_t> _machineOfPeer;

    // Només client:
    PeerId _serverPeer = 0;
    std::vector<uint8_t> _localDogTypes;
    Clock::time_point _connectStart;
};