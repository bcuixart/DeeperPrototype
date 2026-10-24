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
        for (auto& [peer, t] : _pendingPeers) _transport->Disconnect(peer);
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
    _receivedCursors.clear();
    _events.clear();
    _hasSnapshot = false;
    _returnToLobby = false;
    _inputHistory = {};
    _state = State::Idle;
    _timePong.reset();
    _failReason.clear();
    _lastHeard.clear();
    _leftSlots.clear();
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

    if (_isHost && _state == State::InGame)
    {
        const auto now = Clock::now();
        std::vector<std::pair<PeerId, uint8_t>> silent;
        for (const auto& [peer, id] : _machineOfPeer)
        {
            auto it = _lastHeard.find(id);
            if (it == _lastHeard.end()) continue;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > kSilenceTimeoutMs)
                silent.push_back({ peer, id });
        }
        for (auto& [peer, id] : silent)
        {
            _transport->Disconnect(peer);
            HostDropMachine(id);
        }
    }

    if (_isHost && !_pendingPeers.empty())
    {
        const auto now = Clock::now();
        for (auto it = _pendingPeers.begin(); it != _pendingPeers.end();)
        {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count() > kHelloTimeoutMs)
            {
                _transport->Disconnect(it->first);
                it = _pendingPeers.erase(it);
            }
            else ++it;
        }
    }

    if (!_isHost && _state == State::InGame)
    {
        const auto now = Clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastKeepAlive).count() >= kKeepAliveMs)
        {
            _lastKeepAlive = now;
            SendTo(_serverPeer, Channel::Unreliable, EncodeTimePing(NowMs()));
        }
    }
}

bool NetworkManager::HostSetSettings(const LobbySettings& settings)
{
    if (!_isHost || _state != State::InLobby || !settings.IsValid()) return false;
    if (settings == _lobby.settings) return true;

    _lobby.settings = settings;
    BroadcastLobby();
    return true;
}

bool NetworkManager::HostStartLevel(const std::string& levelName, uint32_t roundTicks, uint32_t levelHash,
                                    uint8_t roundIndex, uint8_t roundCount) {
    if (!_isHost || (_state != State::InLobby && _state != State::InGame)) return false;

    static std::mt19937 rng{ std::random_device{}() };

    FreeAbsentSlots();   // qui ha marxat a mitja partida no apareix a la ronda següent

    LevelStartInfo info;
    info.lobby = _lobby;
    info.levelName = levelName;
    info.spawnSeed = rng();
    info.roundTicks = roundTicks;
    info.levelHash = levelHash;
    info.roundIndex = roundIndex;
    info.roundCount = roundCount;
    ResetGameBuffers();
    _leftSlots.clear();

    const auto msg = EncodeStartLevel(info);
    for (const auto& [peer, id] : _machineOfPeer) SendTo(peer, Channel::Reliable, msg);
    for (auto& [peer, id] : _machineOfPeer) _lastHeard[id] = Clock::now();

    _state = State::InGame;           // el lobby es tanca: ningú més pot entrar
    _pendingLevelStart = info;        // el host també ho rep pel mateix camí
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
    if (blob.size() > kMaxSnapshotBytes) { printf("[net] ERROR: snapshot massa gran (%zu bytes)\n", blob.size()); return; }
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
        if (_isHost) _pendingPeers[ev.peer] = Clock::now();   // esperem el Hello
        else if (_state == State::Connecting) {
            _serverPeer = ev.peer;
            SendTo(_serverPeer, Channel::Reliable, EncodeHello(_localDogTypes));
        }
        break;

    case NetEvent::Type::Disconnected:
        if (_isHost) {
            _pendingPeers.erase(ev.peer);
            auto it = _machineOfPeer.find(ev.peer);
            if (it != _machineOfPeer.end()) HostDropMachine(it->second);
        } else if (_state == State::Connecting) {
            Fail("No s'ha pogut connectar");
        } else if (_state == State::InLobby || _state == State::InGame) {
            Fail("El host ha tancat la sala o s'ha perdut la connexió");
        }
        break;

    case NetEvent::Type::Received: {
        ByteReader r(ev.data.data(), ev.data.size());

        if (_isHost)
        {
            auto it = _machineOfPeer.find(ev.peer);
            if (it != _machineOfPeer.end()) _lastHeard[it->second] = Clock::now();
        }

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

        _pendingPeers.erase(peer);

        auto RejectAndKick = [&](RejectReason reason) {
            SendTo(peer, Channel::Reliable, EncodeReject(reason));
            _transport->Disconnect(peer);
        };

        if (_state == State::InGame)                    { RejectAndKick(RejectReason::GameInProgress); return; }
        if (h.version != kProtocolVersion)              { RejectAndKick(RejectReason::VersionMismatch); return; }
        if (h.dogTypes.empty())                         { RejectAndKick(RejectReason::BadRequest); return; }
        for (uint8_t t : h.dogTypes) if (t >= kNumDogTypes) { RejectAndKick(RejectReason::BadRequest); return; }

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
    else if (type == MsgType::GameEvent)
    {
        auto it = _machineOfPeer.find(peer);
        uint8_t kind; std::vector<uint8_t> payload;
        if ((_state == State::InLobby || _state == State::InGame) && it != _machineOfPeer.end() && DecodeGameEvent(r, kind, payload))
            PushEvent(GameEventMsg{ kind, it->second, std::move(payload) });
    }
    else if (type == MsgType::Cursors)
    {
        HostHandleCursors(peer, r);
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
        if (DecodeStartLevel(r, info) && (_state == State::InLobby || _state == State::InGame)) {
            _lobby = info.lobby;
            _state = State::InGame;
            _pendingLevelStart = info;
            ResetGameBuffers();
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
    case MsgType::GameEvent: {
        uint8_t kind; std::vector<uint8_t> payload;
        if ((_state == State::InLobby || _state == State::InGame) && DecodeGameEvent(r, kind, payload))
            PushEvent(GameEventMsg{ kind, 0, std::move(payload) });
        break;
    }
    case MsgType::ReturnToLobby:
        if (_state == State::InGame) { _state = State::InLobby; ResetGameBuffers(); _returnToLobby = true; }
        break;
    case MsgType::Cursors: {
        CursorPacket packet;
        if ((_state == State::InLobby || _state == State::InGame) && DecodeCursors(r, packet))
            PushReceivedCursors(std::move(packet));
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

void NetworkManager::HostBroadcastEvent(uint8_t kind, const std::vector<uint8_t>& payload) 
{
    if (!_isHost || (_state != State::InLobby && _state != State::InGame)) return;
    if (payload.size() > kMaxEventBytes) { printf("[net] ERROR: esdeveniment massa gran\n"); return; }
    SendToAll(Channel::Reliable, EncodeGameEvent(kind, payload));
}

void NetworkManager::SendEventToHost(uint8_t kind, const std::vector<uint8_t>& payload) 
{
    if (_isHost || (_state != State::InLobby && _state != State::InGame)) return;
    if (payload.size() > kMaxEventBytes) { printf("[net] ERROR: esdeveniment massa gran\n"); return; }
    SendTo(_serverPeer, Channel::Reliable, EncodeGameEvent(kind, payload));
}

bool NetworkManager::PopEvent(GameEventMsg& out) 
{
    if (_events.empty()) return false;
    out = std::move(_events.front());
    _events.pop_front();
    return true;
}

void NetworkManager::PushEvent(GameEventMsg&& ev) 
{
    if (_events.size() >= 256) _events.pop_front();
    _events.push_back(std::move(ev));
}

bool NetworkManager::PopLeftSlot(uint8_t& slot)
{
    if (_leftSlots.empty()) return false;
    slot = _leftSlots.front();
    _leftSlots.pop_front();
    return true;
}

void NetworkManager::HostDropMachine(uint8_t machineId)
{
    if (_state == State::InLobby)
    {
        FreeSlots(machineId);
        BroadcastLobby();
    }
    else
    {
        for (const SlotInfo& s : _lobby.slots)
            if (s.machineId == machineId) _leftSlots.push_back((uint8_t)(&s - &_lobby.slots[0]));
    }
    for (auto it = _machineOfPeer.begin(); it != _machineOfPeer.end(); ++it)
        if (it->second == machineId) { _machineOfPeer.erase(it); break; }
    _lastHeard.erase(machineId);
}

void NetworkManager::ResetGameBuffers()
{
    _receivedInputs.clear();
    _receivedCursors.clear();
    _events.clear();
    _inputHistory = {};
    _hasSnapshot = false;
    _timePong.reset();
}

void NetworkManager::FreeAbsentSlots()
{
    for (SlotInfo& s : _lobby.slots)
    {
        if (!s.Occupied() || s.machineId == 0) continue;   // el host no és a _machineOfPeer
        bool connected = false;
        for (const auto& [peer, id] : _machineOfPeer)
            if (id == s.machineId) { connected = true; break; }
        if (!connected) s = SlotInfo{};
    }
}

bool NetworkManager::HostReturnToLobby()
{
    if (!_isHost || _state != State::InGame) return false;
    FreeAbsentSlots();
    ResetGameBuffers();
    _leftSlots.clear();
    _state = State::InLobby;

    const auto msg = EncodeReturnToLobby();
    for (const auto& [peer, id] : _machineOfPeer) SendTo(peer, Channel::Reliable, msg);
    BroadcastLobby();   // després del ReturnToLobby: el canal fiable manté l'ordre
    return true;
}

bool NetworkManager::PopReturnToLobby()
{
    const bool r = _returnToLobby;
    _returnToLobby = false;
    return r;
}

void NetworkManager::SendLocalCursors(const std::vector<CursorState>& cursors)
{
    if (cursors.empty() || (_state != State::InLobby && _state != State::InGame)) return;
    CursorPacket packet;
    packet.cursors = cursors;
    SendToAll(Channel::Unreliable, EncodeCursors(packet));
}

bool NetworkManager::PopReceivedCursors(CursorPacket& out)
{
    if (_receivedCursors.empty()) return false;
    out = std::move(_receivedCursors.front());
    _receivedCursors.pop_front();
    return true;
}

void NetworkManager::PushReceivedCursors(CursorPacket&& packet)
{
    if (_receivedCursors.size() >= 64) _receivedCursors.pop_front();
    _receivedCursors.push_back(std::move(packet));
}

void NetworkManager::HostHandleCursors(PeerId peer, ByteReader& r)
{
    if (_state != State::InLobby && _state != State::InGame) return;
    auto it = _machineOfPeer.find(peer);
    if (it == _machineOfPeer.end()) return;

    CursorPacket packet;
    if (!DecodeCursors(r, packet)) return;

    // Només acceptem cursors dels slots que són realment d'aquesta màquina.
    std::vector<CursorState> valid;
    for (const CursorState& c : packet.cursors)
        if (_lobby.slots[c.slot].machineId == it->second) valid.push_back(c);
    packet.cursors = std::move(valid);
    if (packet.cursors.empty()) return;

    SendToAll(Channel::Unreliable, EncodeCursors(packet), peer);   // reenviem als altres clients
    PushReceivedCursors(std::move(packet));
}

bool NetworkManager::HostSetDogType(uint8_t slot, uint8_t dogType)
{
    if (!_isHost || _state != State::InLobby) return false;
    if (slot >= kMaxSlots || !_lobby.slots[slot].Occupied() || dogType >= kNumDogTypes) return false;
    if (_lobby.slots[slot].dogType == dogType) return true;

    _lobby.slots[slot].dogType = dogType;
    BroadcastLobby();
    return true;
}