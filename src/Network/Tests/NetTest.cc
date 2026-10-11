#include "../Transport/EnetTransport.hh"
#include "../Transport/LagSimTransport.hh"
#include "../Protocol/ByteStream.hh"
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

int main(int argc, char** argv)
{
    const bool isHost = (argc > 1 && std::strcmp(argv[1], "--host") == 0);

    LagSimTransport net = LagSimTransport(std::make_unique<EnetTransport>(), LagSimSettings{ /*latencyMs*/ 100, /*jitterMs*/ 30, /*lossChance*/ 0.0f });;
    PeerId remote = kInvalidPeer;

    if (isHost) { net.StartHost(7777, 8); printf("Host escoltant al port 7777\n"); }
    else        { net.Connect("127.0.0.1", 7777); printf("Connectant...\n"); }

    for (int frame = 0; frame < 60 * 40; frame++)
    {
        std::vector<NetEvent> events;
        net.Poll(events);

        for (NetEvent& e : events)
        {
            if (e.type == NetEvent::Type::Connected)
            {
                printf("Connectat: peer %d\n", e.peer);
                remote = e.peer;
                if (!isHost)
                {
                    ByteWriter w; w.U8(1); w.U32(12345);
                    net.Send(remote, Channel::Reliable, w.Data().data(), w.Data().size());
                }
            }
            else if (e.type == NetEvent::Type::Disconnected)
            {
                printf("Desconnectat: peer %d\n", e.peer);
            }
            else // Received
            {
                ByteReader r(e.data.data(), e.data.size());
                uint8_t type = r.U8();
                uint32_t value = r.U32();
                printf("Rebut de peer %d: tipus %d valor %u (ok=%d, rtt=%d ms)\n",
                       e.peer, type, value, r.Ok(), net.GetRttMs(e.peer));

                if (isHost && type == 1)
                {
                    ByteWriter w; w.U8(2); w.U32(value + 1);
                    net.Send(e.peer, Channel::Reliable, w.Data().data(), w.Data().size());
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (!isHost && remote != kInvalidPeer)
    {
        net.Disconnect(remote);
        for (int i = 0; i < 10; i++)
        {
            std::vector<NetEvent> events;
            net.Poll(events);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    return 0;
}