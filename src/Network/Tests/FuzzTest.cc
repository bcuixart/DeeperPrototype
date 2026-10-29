#include <cstdio>
#include <cstdlib>
#include <functional>
#include <random>
#include <vector>
#include "Network/Protocol/NetworkProtocol.hh"
#include "Level/RoundManager.hh"
#include "Level/MatchManager.hh"
#include "Menu/LobbyEvents.hh"

#define CHECK(c) do { if (!(c)) { printf("CHECK fallat: %s (linia %d)\n", #c, __LINE__); abort(); } } while (0)

static std::mt19937 rng{ 12345 };

static std::vector<uint8_t> RandomBytes()
{
    static const uint8_t special[] = { 0, 1, 2, 7, 8, 9, 10, 255 };   // valors a prop dels límits
    std::vector<uint8_t> v(rng() % 300);
    for (uint8_t& b : v) b = (rng() % 2) ? special[rng() % sizeof(special)] : (uint8_t)rng();
    return v;
}

int main()
{
    const std::vector<std::function<void(ByteReader&)>> decoders = {
        [](ByteReader& r) { MsgHello m; DecodeHello(r, m); },
        [](ByteReader& r) { uint8_t id; DecodeWelcome(r, id); },
        [](ByteReader& r) { RejectReason x; DecodeReject(r, x); },
        [](ByteReader& r) { LobbyState l; if (DecodeLobbyState(r, l)) CHECK(l.settings.IsValid()); },
        [](ByteReader& r) { CursorPacket p; if (DecodeCursors(r, p)) for (auto& c : p.cursors) CHECK(c.slot < kMaxSlots && c.pose < kNumCursorPoses); },
        [](ByteReader& r) { LevelStartInfo i; if (DecodeStartLevel(r, i)) CHECK(i.roundIndex < i.roundCount && !i.levelName.empty()); },
        [](ByteReader& r) { InputPacket p; if (DecodeInput(r, p)) for (auto& s : p.slots) CHECK(s.slot < kMaxSlots && s.count <= kInputRedundancy); },
        [](ByteReader& r) { uint32_t a; DecodeTimePing(r, a); },
        [](ByteReader& r) { uint32_t a, b; DecodeTimePong(r, a, b); },
        [](ByteReader& r) { uint32_t t; std::vector<uint8_t> b; if (DecodeSnapshot(r, t, b)) CHECK(b.size() <= kMaxSnapshotBytes); },
        [](ByteReader& r) { uint8_t k; std::vector<uint8_t> p; DecodeGameEvent(r, k, p); },
        [](ByteReader& r) { uint8_t s; uint16_t v; if (DecodeScoreChanged(r, s, v)) CHECK(s < kMaxSlots); },
        [](ByteReader& r) { uint32_t t; std::array<uint16_t, kMaxSlots> s; DecodeRoundEnd(r, t, s); },
        [](ByteReader& r) { uint8_t s; if (DecodePlayerLeft(r, s)) CHECK(s < kMaxSlots); },
        [](ByteReader& r) { MatchSummary m; if (DecodeMatchSummary(r, m)) CHECK(m.roundIndex < m.roundCount); },
        [](ByteReader& r) { uint8_t s, l; if (DecodePickLevel(r, s, l)) CHECK(s < kMaxSlots); },
        [](ByteReader& r) { uint8_t s, d; if (DecodeSetDogType(r, s, d)) CHECK(s < kMaxSlots && d < kNumDogTypes); },
    };

    for (int iter = 0; iter < 2000000; ++iter)
    {
        const std::vector<uint8_t> data = RandomBytes();
        for (const auto& decode : decoders)
        {
            ByteReader r(data.data(), data.size());
            decode(r);
        }
    }
    printf("OK: cap decoder ha petat\n");
    return 0;
}