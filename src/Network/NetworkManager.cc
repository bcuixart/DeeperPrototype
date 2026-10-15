#include "NetworkManager.hh"
#include <chrono>
#include <random>
#include <thread>

NetworkManager::NetworkManager(std::unique_ptr<ITransport> transport)
    : _transport(std::move(transport)) {}

bool NetworkManager::Host(uint16_t port, const std::vector<uint8_t>& dogTypes) {
    Leave();
    if (dogTypes.empty() || dogTypes.size() > kMaxPlayersPerMachine) return false;
    if (!_transport->StartHost(port, kMaxSlots - 1)) return false;
    _isHost = true;
    _myMachineId = 0;
    _lobby = LobbyState{};
    AllocSlots(0, dogTypes);
    _state = State::InLobby;
    return true;
}

void NetworkManager::Join(const std::string& address, uint16_t port, const std::vector<uint8_t>& dogTypes) {
    Leave();
    _localDogTypes = dogTypes;
    if (dogTypes.empty() || dogTypes.size() > kMaxPlayersPerMachine) { Fail("Nombre de jugadors invàlid"); return; }
    if (!_transport->Connect(address, port)) { Fail("No s'ha pogut iniciar la connexió"); return; }
    _state = State::Connecting;
    _connectStart = Clock::now();
}

void NetworkManager::Leave() {
    if (_isHost) {
        for (auto& [peer, id] : _machineOfPeer) _transport->Disconnect(peer);
    } else if (_state == State::Connecting || _state == State::InLobby || _state == State::InGame) {
        _transport->Disconnect(_serverPeer);
    }
    // Deixem uns instants perquè s'enviïn els avisos de desconnexió.
    // (Temporal: bloqueja ~200 ms. Només passa en sortir/canviar de lobby.)
    std::vector<NetEvent> dump;
    for (int i = 0; i < 20; ++i) {
        _transport->Poll(dump);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    _machineOfPeer.clear();
    _lobby = LobbyState{};
    _isHost = false;
    _myMachineId = kNoMachine;
    _pendingLevelStart.reset();
    _receivedInputs.clear(); 
    _hasSnapshot = false;
    _inputHistory = {};
    _state = State::Idle;
    _timePong.reset();
    _failReason.clear();
}

void NetworkManager::Update() {
    std::vector<NetEvent> events;
    _transport->Poll(events);
    for (const NetEvent& ev : events) HandleEvent(ev);

    // El timeout de connexió el portem nosaltres, no ENet.
    if (_state == State::Connecting) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _connectStart);
        if (elapsed.count() > kConnectTimeoutMs) {
            _transport->Disconnect(_serverPeer);
            Fail("Temps d'espera esgotat");
        }
    }
}

bool NetworkManager::HostStartLevel(const std::string& levelName) {
    if (!_isHost || _state != State::InLobby) return false;

    static std::mt19937 rng{ std::random_device{}() };

    LevelStartInfo info;
    info.lobby = _lobby;
    info.levelName = levelName;
    info.spawnSeed = rng();

    const auto msg = EncodeStartLevel(info);
    for (const auto& [peer, id] : _machineOfPeer) SendTo(peer, Channel::Reliable, msg);

    _state = State::InGame;           // el lobby es tanca: ningú més pot entrar
    _pendingLevelStart = info;        // el host també ho rep pel mateix camí
    _timePong.reset();
    return true;
}

bool NetworkManager::PopLevelStart(LevelStartInfo& out) {
    if (!_pendingLevelStart) return false;
    out = *_pendingLevelStart;
    _pendingLevelStart.reset();
    return true;
}

void NetworkManager::SendLocalInputs(uint32_t tick, const std::vector<uint8_t>& mySlots, const std::array<uint8_t, kMaxSlots>& slotInput)
{
    if (_state != State::InGame) return;

    InputPacket packet;
    packet.tick = tick;
    for (uint8_t slot : mySlots)
    {
        InputHistory& h = _inputHistory[slot];
        // Desplacem l'historial una posició i posem l'input d'aquest tick al davant.
        for (int i = kInputRedundancy - 1; i > 0; --i) h.inputs[i] = h.inputs[i - 1];
        h.inputs[0] = slotInput[slot];
        if (h.count < kInputRedundancy) ++h.count;

        SlotInputs s;
        s.slot = slot;
        s.count = h.count;
        s.inputs = h.inputs;
        packet.slots.push_back(s);
    }
    if (packet.slots.empty()) return;

    SendToAll(Channel::Unreliable, EncodeInput(packet));
}

bool NetworkManager::PopReceivedInput(InputPacket& out)
{
    if (_receivedInputs.empty()) return false;
    out = std::move(_receivedInputs.front());
    _receivedInputs.pop_front();
    return true;
}

int NetworkManager::GetRttMs() const
{
    return _isHost ? 0 : _transport->GetRttMs(_serverPeer);
}

uint32_t NetworkManager::NowMs() const
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _epoch).count();
}

void NetworkManager::SendTimePing()
{
    if (_isHost || _state != State::InGame) return;
    SendTo(_serverPeer, Channel::Unreliable, EncodeTimePing(NowMs()));
}

bool NetworkManager::PopTimePong(uint32_t& hostTick, int& rttMs)
{
    if (!_timePong) return false;
    hostTick = _timePong->hostTick;
    rttMs = _timePong->rttMs;
    _timePong.reset();
    return true;
}

void NetworkManager::HostSendSnapshot(uint32_t tick, const std::vector<uint8_t>& blob) {
    if (!IsHost() || _state != State::InGame) return;
    SendToAll(Channel::Unreliable, EncodeSnapshot(tick, blob), kInvalidPeer);
}

bool NetworkManager::PopSnapshot(uint32_t& tick, std::vector<uint8_t>& blob) {
    if (!_hasSnapshot) return false;
    tick = _snapTick;
    blob = std::move(_snapBlob);
    _hasSnapshot = false;
    return true;
}


std::vector<uint8_t> NetworkManager::GetMySlots() const {
    std::vector<uint8_t> mine;
    for (int s = 0; s < kMaxSlots; ++s)
        if (_lobby.slots[s].machineId == _myMachineId) mine.push_back((uint8_t)s);
    return mine;
}

void NetworkManager::HandleEvent(const NetEvent& ev) {
    switch (ev.type) {
    case NetEvent::Type::Connected:
        if (!_isHost && _state == State::Connecting) {
            _serverPeer = ev.peer;
            SendTo(_serverPeer, Channel::Reliable, EncodeHello(_localDogTypes));
        }
        // Host: esperem el Hello abans de fer res amb aquest peer.
        break;

    case NetEvent::Type::Disconnected:
        if (_isHost) {
            auto it = _machineOfPeer.find(ev.peer);
            if (it != _machineOfPeer.end()) {
                if (_state == State::InLobby) {   // en partida no s'alliberen: ho tractarem més endavant
                    FreeSlots(it->second);
                    _machineOfPeer.erase(it);
                    BroadcastLobby();
                } else {
                    _machineOfPeer.erase(it);
                }
            }
        } else if (_state == State::Connecting) {
            Fail("No s'ha pogut connectar");
        } else if (_state == State::InLobby || _state == State::InGame) {
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

void NetworkManager::HostHandleMessage(PeerId peer, ByteReader& r) {
    MsgType type = (MsgType)r.U8();
    if (!r.Ok()) return;

    if (type == MsgType::Hello) {
        if (_machineOfPeer.count(peer)) return;   // ja ha fet Hello
        MsgHello h;
        if (!DecodeHello(r, h)) return;

        auto RejectAndKick = [&](RejectReason reason) {
            SendTo(peer, Channel::Reliable, EncodeReject(reason));
            _transport->Disconnect(peer);
        };

        if (_state == State::InGame)                    { RejectAndKick(RejectReason::GameInProgress); return; }
        if (h.version != kProtocolVersion)              { RejectAndKick(RejectReason::VersionMismatch); return; }
        if (h.dogTypes.empty())                         { RejectAndKick(RejectReason::BadRequest); return; }
        // TODO: validar que cada dogType sigui un tipus de gos que existeix.

        uint8_t id = NextFreeMachineId();
        if (id == kNoMachine || !AllocSlots(id, h.dogTypes)) { RejectAndKick(RejectReason::LobbyFull); return; }

        _machineOfPeer[peer] = id;
        SendTo(peer, Channel::Reliable, EncodeWelcome(id));
        BroadcastLobby();
    }
    else if (type == MsgType::Input) 
    {
        HostHandleInput(peer, r);
    }
    else if (type == MsgType::TimePing) 
    {
        uint32_t sentMs;
        if (_state == State::InGame && _machineOfPeer.count(peer) && DecodeTimePing(r, sentMs))
            SendTo(peer, Channel::Unreliable, EncodeTimePong(sentMs, _localTick));
    }
    // Altres missatges d'un peer sense Hello es descarten.
}

void NetworkManager::ClientHandleMessage(ByteReader& r) {
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
        case RejectReason::GameInProgress:  Fail("La partida ja ha començat"); break;
        default:                            Fail("Sol·licitud rebutjada"); break;
        }
        break;
    }
    case MsgType::LobbyState: {
        LobbyState l;
        if (DecodeLobbyState(r, l) && _state == State::InLobby) _lobby = l;
        break;
    }
    case MsgType::StartLevel: {
        LevelStartInfo info;
        if (DecodeStartLevel(r, info) && _state == State::InLobby) {
            _lobby = info.lobby;
            _state = State::InGame;
            _pendingLevelStart = info;
            _receivedInputs.clear(); 
            _timePong.reset();
            _inputHistory = {};
            _hasSnapshot = false;
        }
        break;
    }
    case MsgType::Input: {
        InputPacket packet;
        if (_state == State::InGame && DecodeInput(r, packet)) PushReceivedInput(std::move(packet));
        break;
    }
    case MsgType::TimePong: {
        uint32_t echoMs, hostTick;
        if (_state == State::InGame && DecodeTimePong(r, echoMs, hostTick))
            _timePong = TimePong{ hostTick, (int)(NowMs() - echoMs) };
        break;
    }
    case MsgType::Snapshot: {
        uint32_t tick; std::vector<uint8_t> blob;
        if (!DecodeSnapshot(r, tick, blob)) break;
        if (!_hasSnapshot || static_cast<int32_t>(tick - _snapTick) > 0) {
            _snapTick = tick; _snapBlob = std::move(blob); _hasSnapshot = true;
        }
        break;
    }
    default: break;
    }
}

bool NetworkManager::AllocSlots(uint8_t machineId, const std::vector<uint8_t>& dogTypes) {
    int freeCount = 0;
    for (const SlotInfo& s : _lobby.slots) if (!s.Occupied()) ++freeCount;
    if (freeCount < (int)dogTypes.size()) return false;

    size_t next = 0;
    for (SlotInfo& s : _lobby.slots) {
        if (next == dogTypes.size()) break;
        if (!s.Occupied()) {
            s.machineId = machineId;
            s.dogType = dogTypes[next++];
        }
    }
    return true;
}

void NetworkManager::FreeSlots(uint8_t machineId) {
    for (SlotInfo& s : _lobby.slots) if (s.machineId == machineId) s = SlotInfo{};
}

uint8_t NetworkManager::NextFreeMachineId() const {
    for (uint8_t id = 1; id < kMaxSlots; ++id) {
        bool used = false;
        for (const auto& [peer, m] : _machineOfPeer) if (m == id) { used = true; break; }
        if (!used) return id;
    }
    return kNoMachine;
}

void NetworkManager::BroadcastLobby() {
    auto msg = EncodeLobbyState(_lobby);
    for (const auto& [peer, id] : _machineOfPeer) SendTo(peer, Channel::Reliable, msg);
}

void NetworkManager::SendTo(PeerId peer, Channel ch, const std::vector<uint8_t>& msg) {
    _transport->Send(peer, ch, msg.data(), msg.size());
}

void NetworkManager::Fail(const std::string& reason) {
    _failReason = reason;
    _state = State::Failed;
}

void NetworkManager::SendToAll(Channel ch, const std::vector<uint8_t>& msg, PeerId except)
{
    if (_isHost)
    {
        for (const auto& [peer, id] : _machineOfPeer)
            if (peer != except) SendTo(peer, ch, msg);
    }
    else SendTo(_serverPeer, ch, msg);
}

void NetworkManager::PushReceivedInput(InputPacket&& packet)
{
    if (_receivedInputs.size() >= 256) _receivedInputs.pop_front();   // si el joc no els consumeix, no creix sense límit
    _receivedInputs.push_back(std::move(packet));
}

void NetworkManager::HostHandleInput(PeerId peer, ByteReader& r)
{
    if (_state != State::InGame) return;
    auto it = _machineOfPeer.find(peer);
    if (it == _machineOfPeer.end()) return;

    InputPacket packet;
    if (!DecodeInput(r, packet)) return;

    // Només acceptem slots que pertanyen realment a la màquina que els envia.
    std::vector<SlotInputs> valid;
    for (const SlotInputs& s : packet.slots)
        if (_lobby.slots[s.slot].machineId == it->second) valid.push_back(s);
    packet.slots = std::move(valid);
    if (packet.slots.empty()) return;

    SendToAll(Channel::Unreliable, EncodeInput(packet), peer);   // reenviem als altres clients
    PushReceivedInput(std::move(packet));
}