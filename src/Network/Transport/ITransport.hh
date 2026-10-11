#ifndef ITRANSPORT_HH
#define ITRANSPORT_HH

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

using PeerId = uint16_t;
constexpr PeerId kInvalidPeer = 0xFFFF;

enum class Channel : uint8_t
{
    Reliable,
    Unreliable
};

struct NetEvent
{
    enum class Type { Connected, Disconnected, Received };

    Type type;
    PeerId peer = kInvalidPeer;
    Channel channel = Channel::Reliable;
    std::vector<uint8_t> data;
};

class ITransport
{
public:
    virtual ~ITransport() = default;

    virtual bool StartHost(uint16_t port, int maxPeers) = 0;
    virtual bool Connect(const std::string& address, uint16_t port) = 0;
    virtual void Disconnect(PeerId peer) = 0;

    virtual void Send(PeerId peer, Channel channel, const uint8_t* data, size_t size) = 0;

    virtual void Poll(std::vector<NetEvent>& out) = 0;

    virtual int GetRttMs(PeerId peer) const = 0;
};

#endif