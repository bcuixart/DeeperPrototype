#ifndef NETWORK_PROTOCOL_HH
#define NETWORK_PROTOCOL_HH

#include "../NetworkTypes.hh"
#include "ByteStream.hh"
#include <vector>

enum class MsgType : uint8_t { Hello = 1, Welcome, Reject, LobbyState, StartLevel, Input };
enum class RejectReason : uint8_t { VersionMismatch = 1, LobbyFull, BadRequest, GameInProgress };

struct MsgHello { uint16_t version = 0; uint8_t numPlayers = 0; std::vector<uint8_t> dogTypes; };

std::vector<uint8_t> EncodeHello(const std::vector<uint8_t>& dogTypes);
std::vector<uint8_t> EncodeWelcome(uint8_t machineId);
std::vector<uint8_t> EncodeReject(RejectReason reason);
std::vector<uint8_t> EncodeLobbyState(const LobbyState& lobby);
std::vector<uint8_t> EncodeStartLevel(const LevelStartInfo& info);
std::vector<uint8_t> EncodeInput(const InputPacket& packet);

bool DecodeHello(ByteReader& r, MsgHello& out);
bool DecodeWelcome(ByteReader& r, uint8_t& machineId);
bool DecodeReject(ByteReader& r, RejectReason& out);
bool DecodeLobbyState(ByteReader& r, LobbyState& out);
bool DecodeStartLevel(ByteReader& r, LevelStartInfo& out);
bool DecodeInput(ByteReader& r, InputPacket& out);

#endif