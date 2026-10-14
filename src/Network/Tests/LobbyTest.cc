#include "../NetworkManager.hh"
#include "../Transport/EnetTransport.hh"
#include "../Transport/LagSimTransport.hh"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

static void PrintLobby(const LobbyState& l) 
{
    printf("Lobby:");
    for (const SlotInfo& s : l.slots) {
        if (s.Occupied()) printf(" [M%d]", s.machineId); else printf(" [--]");
    }
    printf("\n");
}

int main(int argc, char** argv) 
{
    // Ús: lobbytest --host [jugadors]   |   lobbytest --join [adreça] [jugadors]
    bool host = argc > 1 && strcmp(argv[1], "--host") == 0;
    bool join = argc > 1 && strcmp(argv[1], "--join") == 0;
    if (!host && !join) { printf("Ús: --host [n] | --join [adreça] [n]\n"); return 1; }

    NetworkManager net(std::make_unique<LagSimTransport>(
        std::make_unique<EnetTransport>(), LagSimSettings{80, 20, 0.0f}));

    auto MakeDogTypes = [](int n) { std::vector<uint8_t> t; for (int i = 0; i < n; ++i) t.push_back((uint8_t)i); return t; };

    if (host) {
        int n = (argc > 2) ? atoi(argv[2]) : 1;
        if (!net.Host(7777, MakeDogTypes(n))) { printf("No puc obrir host\n"); return 1; }
        printf("Host obert amb %d jugador(s) locals\n", n);
    } else {
        const char* addr = (argc > 2) ? argv[2] : "127.0.0.1";
        int n = (argc > 3) ? atoi(argv[3]) : 1;
        net.Join(addr, 7777, MakeDogTypes(n));
        printf("Connectant a %s amb %d jugador(s)...\n", addr, n);
    }

    LobbyState lastLobby;
    NetworkManager::State lastState = NetworkManager::State::Idle;
    bool first = true;

    for (int i = 0; i < 60 * 60; ++i) {   // ~60 s
        net.Update();
        if (host && i == 940) {
            printf("[host] Començant nivell...\n");
            net.HostStartLevel("test_level");
        }

        LevelStartInfo start;
        if (net.PopLevelStart(start)) {
            printf("INICI DE NIVELL '%s' seed=%u\n", start.levelName.c_str(), start.spawnSeed);
            for (int s = 0; s < kMaxSlots; ++s)
                if (start.lobby.slots[s].Occupied())
                    printf("  slot %d: màquina %d, gos %d\n", s, start.lobby.slots[s].machineId, start.lobby.slots[s].dogType);
            auto mine = net.GetMySlots();
            printf("  Els meus slots:");
            for (uint8_t s : mine) printf(" %d", s);
            printf("\n");
        }

        if (first || net.GetState() != lastState) {
            lastState = net.GetState();
            printf("Estat: %d (machineId=%d) %s\n", (int)lastState, net.GetMyMachineId(), net.GetFailReason().c_str());
        }
        if (first || net.GetLobby() != lastLobby) {
            lastLobby = net.GetLobby();
            PrintLobby(lastLobby);
        }
        first = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    net.Leave();
    return 0;
}