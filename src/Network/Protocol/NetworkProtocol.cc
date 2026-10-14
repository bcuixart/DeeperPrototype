#include "NetworkProtocol.hh"

static void WriteLobby(ByteWriter& w, const LobbyState& lobby) 
{
    for (const SlotInfo& s : lobby.slots) { w.U8(s.machineId); w.U8(s.dogType); }
}

static void ReadLobby(ByteReader& r, LobbyState& lobby) 
{
    for (SlotInfo& s : lobby.slots) { s.machineId = r.U8(); s.dogType = r.U8(); }
}

/* ENCODING */

std::vector<uint8_t> EncodeHello(const std::vector<uint8_t>& dogTypes) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::Hello);
    w.U16(kProtocolVersion);
    w.U8((uint8_t)dogTypes.size());
    for (uint8_t t : dogTypes) w.U8(t);
    return w.Data();
}

std::vector<uint8_t> EncodeWelcome(uint8_t machineId) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::Welcome);
    w.U8(machineId);
    return w.Data();
}

std::vector<uint8_t> EncodeReject(RejectReason reason) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::Reject);
    w.U8((uint8_t)reason);
    return w.Data();
}

std::vector<uint8_t> EncodeLobbyState(const LobbyState& lobby) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::LobbyState);
    for (const SlotInfo& s : lobby.slots) w.U8(s.machineId);
    return w.Data();
}

std::vector<uint8_t> EncodeStartLevel(const LevelStartInfo& info) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::StartLevel);
    WriteLobby(w, info.lobby);
    w.U32(info.spawnSeed);
    const uint8_t len = (uint8_t)std::min<size_t>(info.levelName.size(), 255);
    w.U8(len);
    for (uint8_t i = 0; i < len; ++i) w.U8((uint8_t)info.levelName[i]);
    return w.Data();
}

/* DECODING */

bool DecodeHello(ByteReader& r, MsgHello& out) 
{
    out.version = r.U16();
    const uint8_t n = r.U8();
    if (!r.Ok() || n > kMaxPlayersPerMachine) return false;
    out.dogTypes.resize(n);
    for (uint8_t i = 0; i < n; ++i) out.dogTypes[i] = r.U8();
    return r.Ok();
}

bool DecodeWelcome(ByteReader& r, uint8_t& machineId) 
{
    machineId = r.U8();
    return r.Ok();
}

bool DecodeReject(ByteReader& r, RejectReason& out) 
{
    out = (RejectReason)r.U8();
    return r.Ok();
}

bool DecodeLobbyState(ByteReader& r, LobbyState& out) 
{
    for (SlotInfo& s : out.slots) s.machineId = r.U8();
    return r.Ok();
}

bool DecodeStartLevel(ByteReader& r, LevelStartInfo& out) 
{
    ReadLobby(r, out.lobby);
    out.spawnSeed = r.U32();
    const uint8_t len = r.U8();
    if (!r.Ok() || r.Remaining() < len) return false;
    out.levelName.resize(len);
    for (uint8_t i = 0; i < len; ++i) out.levelName[i] = (char)r.U8();
    return r.Ok();
}