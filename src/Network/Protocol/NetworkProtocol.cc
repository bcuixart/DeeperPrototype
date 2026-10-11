#include "NetworkProtocol.hh"

/* ENCODING */

std::vector<uint8_t> EncodeHello(uint8_t numPlayers) 
{
    ByteWriter w;
    w.U8((uint8_t)MsgType::Hello);
    w.U16(kProtocolVersion);
    w.U8(numPlayers);
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

/* DECODING */

bool DecodeHello(ByteReader& r, MsgHello& out) 
{
    out.version = r.U16();
    out.numPlayers = r.U8();
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