#include "EnetTransport.hh"
#include <enet/enet.h>
#include <cstdio>
#include <utility>

EnetTransport::EnetTransport()
{
    _initialized = (enet_initialize() == 0);
    if (!_initialized) printf("[Enet] Enet failed to initialize\n");
}

EnetTransport::~EnetTransport()
{
    if (_host) enet_host_destroy(_host);
    if (_initialized) enet_deinitialize();
}

bool EnetTransport::StartHost(uint16_t port, int maxPeers)
{
    if (!_initialized || _host) return false;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    _host = enet_host_create(&address, maxPeers, kChannelCount, 0, 0);
    return _host != nullptr;
}

bool EnetTransport::Connect(const std::string& address, uint16_t port)
{
    if (!_initialized || _host) return false;

    _host = enet_host_create(nullptr, 1, kChannelCount, 0, 0);
    if (!_host) return false;

    ENetAddress addr;
    if (enet_address_set_host(&addr, address.c_str()) != 0 ||
        enet_host_connect(_host, (addr.port = port, &addr), kChannelCount, 0) == nullptr)
    {
        enet_host_destroy(_host);
        _host = nullptr;
        return false;
    }

    return true;
}

void EnetTransport::Disconnect(PeerId peer)
{
    ENetPeer* p = GetPeer(peer);
    if (p) enet_peer_disconnect_later(p, 0);
}

void EnetTransport::Send(PeerId peer, Channel channel, const uint8_t* data, size_t size)
{
    ENetPeer* p = GetPeer(peer);
    if (!p || p->state != ENET_PEER_STATE_CONNECTED) return;

    const bool reliable = (channel == Channel::Reliable);
    ENetPacket* packet = enet_packet_create(data, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (enet_peer_send(p, reliable ? 0 : 1, packet) < 0) enet_packet_destroy(packet);
}

void EnetTransport::Poll(std::vector<NetEvent>& out)
{
    if (!_host) return;

    ENetEvent event;
    while (enet_host_service(_host, &event, 0) > 0)
    {
        NetEvent ev;
        ev.peer = event.peer->incomingPeerID;

        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            ev.type = NetEvent::Type::Connected;
            out.push_back(std::move(ev));
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            ev.type = NetEvent::Type::Disconnected;
            out.push_back(std::move(ev));
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            ev.type = NetEvent::Type::Received;
            ev.channel = (event.channelID == 0) ? Channel::Reliable : Channel::Unreliable;
            ev.data.assign(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);
            out.push_back(std::move(ev));
            break;

        default:
            break;
        }
    }
}

int EnetTransport::GetRttMs(PeerId peer) const
{
    ENetPeer* p = GetPeer(peer);
    return p ? (int)p->roundTripTime : -1;
}

ENetPeer* EnetTransport::GetPeer(PeerId id) const
{
    if (!_host || id >= _host->peerCount) return nullptr;
    return &_host->peers[id];
}