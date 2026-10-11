#include "NetworkManager.hh"
#include <chrono>
#include <thread>

NetworkManager::NetworkManager(std::unique_ptr<ITransport> transport)
    : _transport(std::move(transport)) {}

bool NetworkManager::Host(uint16_t port, int localPlayers) 
{
    Leave();
    if (localPlayers < 1 || localPlayers > kMaxPlayersPerMachine) return false;
    if (!_transport->StartHost(port, kMaxSlots - 1)) return false;
    _isHost = true;
    _myMachineId = 0;
    _lobby = LobbyState{};
    AllocSlots(0, localPlayers);
    _state = State::InLobby;
    return true;
}

void NetworkManager::Join(const std::string& address, uint16_t port, int localPlayers) 
{
    Leave();
    _requestedPlayers = localPlayers;
    if (!_transport->Connect(address, port)) { Fail("No s'ha pogut iniciar la connexió"); return; }
    _state = State::Connecting;
    _connectStart = Clock::now();
}

void NetworkManager::Leave() 
{
    if (_isHost) {
        for (auto& [peer, id] : _machineOfPeer) _transport->Disconnect(peer);
    } else if (_state == State::Connecting || _state == State::InLobby) {
        _transport->Disconnect(_serverPeer);
    }
    // Deixem uns instants perquè s'enviïn els avisos de desconnexió.
    // (Temporal: bloqueja ~100 ms. Només passa en sortir/canviar de lobby.)
    std::vector<NetEvent> dump;
    for (int i = 0; i < 10; ++i) 
    {
        _transport->Poll(dump);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    _machineOfPeer.clear();
    _lobby = LobbyState{};
    _isHost = false;
    _myMachineId = kNoMachine;
    _state = State::Idle;
    _failReason.clear();
}

void NetworkManager::Update() 
{
    std::vector<NetEvent> events;
    _transport->Poll(events);
    for (const NetEvent& ev : events) HandleEvent(ev);

    // El timeout de connexió el portem nosaltres, no ENet.
    if (_state == State::Connecting) 
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _connectStart);
        if (elapsed.count() > kConnectTimeoutMs) 
        {
            _transport->Disconnect(_serverPeer);
            Fail("Temps d'espera esgotat");
        }
    }
}

void NetworkManager::HandleEvent(const NetEvent& ev) 
{
    switch (ev.type) 
    {
    case NetEvent::Type::Connected:
        if (!_isHost && _state == State::Connecting) {
            _serverPeer = ev.peer;
            SendTo(_serverPeer, Channel::Reliable, EncodeHello((uint8_t)_requestedPlayers));
        }
        // Host: esperem el Hello abans de fer res amb aquest peer.
        break;

    case NetEvent::Type::Disconnected:
        if (_isHost) {
            auto it = _machineOfPeer.find(ev.peer);
            if (it != _machineOfPeer.end()) {
                FreeSlots(it->second);
                _machineOfPeer.erase(it);
                BroadcastLobby();
            }
        } else if (_state == State::Connecting) {
            Fail("No s'ha pogut connectar");
        } else if (_state == State::InLobby) {
            Fail("El host ha tancat la sala o s'ha perdut la connexió");
        }
        break;

    case NetEvent::Type::Received: {
        ByteReader r(ev.data.data(), ev.data.size());
        if (_isHost) HostHandleMessage(ev.peer, r);
        else         ClientHandleMessage(r);
        break;
    }
    }
}

void NetworkManager::HostHandleMessage(PeerId peer, ByteReader& r) 
{
    MsgType type = (MsgType)r.U8();
    if (!r.Ok()) return;

    if (type == MsgType::Hello) {
        if (_machineOfPeer.count(peer)) return;   // ja ha fet Hello
        MsgHello h;
        if (!DecodeHello(r, h)) return;

        if (h.version != kProtocolVersion) {
            SendTo(peer, Channel::Reliable, EncodeReject(RejectReason::VersionMismatch));
            _transport->Disconnect(peer);
            return;
        }
        if (h.numPlayers < 1 || h.numPlayers > kMaxPlayersPerMachine) {
            SendTo(peer, Channel::Reliable, EncodeReject(RejectReason::BadRequest));
            _transport->Disconnect(peer);
            return;
        }
        uint8_t id = NextFreeMachineId();
        if (id == kNoMachine || !AllocSlots(id, h.numPlayers)) {
            SendTo(peer, Channel::Reliable, EncodeReject(RejectReason::LobbyFull));
            _transport->Disconnect(peer);
            return;
        }
        _machineOfPeer[peer] = id;
        SendTo(peer, Channel::Reliable, EncodeWelcome(id));
        BroadcastLobby();
    }
    // Altres missatges d'un peer sense Hello es descarten.
}

void NetworkManager::ClientHandleMessage(ByteReader& r) 
{
    MsgType type = (MsgType)r.U8();
    if (!r.Ok()) return;

    switch (type) {
    case MsgType::Welcome: {
        uint8_t id;
        if (DecodeWelcome(r, id) && _state == State::Connecting) {
            _myMachineId = id;
            _state = State::InLobby;
        }
        break;
    }
    case MsgType::Reject: {
        RejectReason reason;
        if (!DecodeReject(r, reason)) return;
        switch (reason) {
        case RejectReason::VersionMismatch: Fail("Versió del joc incompatible"); break;
        case RejectReason::LobbyFull:       Fail("La sala està plena"); break;
        default:                            Fail("Sol·licitud rebutjada"); break;
        }
        break;
    }
    case MsgType::LobbyState: {
        LobbyState l;
        if (DecodeLobbyState(r, l)) _lobby = l;
        break;
    }
    default: break;
    }
}

bool NetworkManager::AllocSlots(uint8_t machineId, int count) 
{
    int freeCount = 0;
    for (const SlotInfo& s : _lobby.slots) if (!s.Occupied()) ++freeCount;
    if (freeCount < count) return false;
    for (SlotInfo& s : _lobby.slots) {
        if (count == 0) break;
        if (!s.Occupied()) { s.machineId = machineId; --count; }
    }
    return true;
}

void NetworkManager::FreeSlots(uint8_t machineId) 
{
    for (SlotInfo& s : _lobby.slots) if (s.machineId == machineId) s = SlotInfo{};
}

uint8_t NetworkManager::NextFreeMachineId() const 
{
    for (uint8_t id = 1; id < kMaxSlots; ++id) {
        bool used = false;
        for (const auto& [peer, m] : _machineOfPeer) if (m == id) { used = true; break; }
        if (!used) return id;
    }
    return kNoMachine;
}

void NetworkManager::BroadcastLobby() 
{
    auto msg = EncodeLobbyState(_lobby);
    for (const auto& [peer, id] : _machineOfPeer) SendTo(peer, Channel::Reliable, msg);
}

void NetworkManager::SendTo(PeerId peer, Channel ch, const std::vector<uint8_t>& msg) 
{
    _transport->Send(peer, ch, msg.data(), msg.size());
}

void NetworkManager::Fail(const std::string& reason) 
{
    _failReason = reason;
    _state = State::Failed;
}