#ifndef NETWORK_PROTOCOL_HH
#define NETWORK_PROTOCOL_HH

#include "../NetworkTypes.hh"
#include "ByteStream.hh"
#include <vector>

enum class MsgType : uint8_t { Hello = 1, Welcome, Reject, LobbyState, Cursors, StartLevel, Input, TimePing, TimePong, Snapshot, GameEvent, ReturnToLobby };
enum class RejectReason : uint8_t { VersionMismatch = 1, LobbyFull, BadRequest, GameInProgress };

struct MsgHello { uint16_t version = 0; uint8_t numPlayers = 0; std::vector<uint8_t> dogTypes; };

std::vector<uint8_t> EncodeHello(const std::vector<uint8_t>& dogTypes);
std::vector<uint8_t> EncodeWelcome(uint8_t machineId);
std::vector<uint8_t> EncodeReject(RejectReason reason);
std::vector<uint8_t> EncodeLobbyState(const LobbyState& lobby);
std::vector<uint8_t> EncodeCursors(const CursorPacket& packet);
std::vector<uint8_t> EncodeStartLevel(const LevelStartInfo& info);
std::vector<uint8_t> EncodeInput(const InputPacket& packet);
std::vector<uint8_t> EncodeTimePing(uint32_t sentMs);
std::vector<uint8_t> EncodeTimePong(uint32_t echoMs, uint32_t hostTick);
std::vector<uint8_t> EncodeSnapshot(uint32_t tick, const std::vector<uint8_t>& blob);
std::vector<uint8_t> EncodeGameEvent(uint8_t kind, const std::vector<uint8_t>& payload);
std::vector<uint8_t> EncodeReturnToLobby();

bool DecodeHello(ByteReader& r, MsgHello& out);
bool DecodeWelcome(ByteReader& r, uint8_t& machineId);
bool DecodeReject(ByteReader& r, RejectReason& out);
bool DecodeLobbyState(ByteReader& r, LobbyState& out);
bool DecodeCursors(ByteReader& r, CursorPacket& out);
bool DecodeStartLevel(ByteReader& r, LevelStartInfo& out);
bool DecodeInput(ByteReader& r, InputPacket& out);
bool DecodeTimePing(ByteReader& r, uint32_t& sentMs);
bool DecodeTimePong(ByteReader& r, uint32_t& echoMs, uint32_t& hostTick);
bool DecodeSnapshot(ByteReader& r, uint32_t& tick, std::vector<uint8_t>& blob);
bool DecodeGameEvent(ByteReader& r, uint8_t& kind, std::vector<uint8_t>& payload);

#endif