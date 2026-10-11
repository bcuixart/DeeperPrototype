#ifndef ENETTRANSPORT_HH
#define ENETTRANSPORT_HH

#include "ITransport.hh"

// Forward declaration of ENet structures
// This avoids including the ENet header directly in this header file,
// because it has conflicting definitions with Windows headers.
struct _ENetHost;
struct _ENetPeer;

class EnetTransport : public ITransport
{
public:
    EnetTransport();
    ~EnetTransport() override;

    bool StartHost(uint16_t port, int maxPeers) override;
    bool Connect(const std::string& address, uint16_t port) override;
    void Disconnect(PeerId peer) override;

    void Send(PeerId peer, Channel channel, const uint8_t* data, size_t size) override;

    void Poll(std::vector<NetEvent>& out) override;

    int  GetRttMs(PeerId peer) const override;

private:
    _ENetPeer* GetPeer(PeerId id) const;

    _ENetHost* _host = nullptr;
    bool _initialized = false;

    static constexpr int kChannelCount = 2;   // 0: Reliable, 1: Unreliable
};

#endif