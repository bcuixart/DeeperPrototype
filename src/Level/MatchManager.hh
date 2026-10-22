#ifndef MATCHMANAGER_HH
#define MATCHMANAGER_HH

#include <array>
#include <cstdint>
#include <vector>
#include "Network/NetworkTypes.hh"
#include "Network/Protocol/ByteStream.hh"

static_assert(kMaxSlots == 8, "Les màscares de slots de MatchSummary són uint8_t");

constexpr uint8_t kNoWinner = 0xFF;

// Tipus d'esdeveniment de partida. Comparteixen espai amb RoundEvent (1..3): no es poden repetir números.
enum class MatchEvent : uint8_t { Summary = 4, PickLevel = 5 };

struct MatchSummary
{
    uint8_t roundIndex = 0;          // ronda que s'acaba de jugar (des de 0)
    uint8_t roundCount = 1;
    uint8_t roundWinner = kNoWinner;
    uint8_t matchWinner = kNoWinner; // només té sentit si isFinal
    bool    isFinal = false;
    uint8_t playedMask = 0;          // slots que han jugat alguna ronda
    uint8_t presentMask = 0;         // slots que hi són encara (no han marxat)
    std::array<uint16_t, kMaxSlots> lastRound{};
    std::array<uint16_t, kMaxSlots> totals{};
};

class MatchManager {
public:
    static constexpr uint8_t kDefaultRoundCount = 3;   // quan hi hagi opcions al lobby: 10 per defecte

    void Reset(uint8_t roundCount);                       // host, en començar una partida
    void SetRound(uint8_t index, uint8_t count);          // tothom, en començar cada nivell

    // Només host: suma la ronda, decideix el guanyador de la ronda i, si era l'última, el de la partida.
    void EndRound(const std::array<uint16_t, kMaxSlots>& scores,
                  const std::array<bool, kMaxSlots>& active,
                  const std::array<bool, kMaxSlots>& gone);

    // Només clients: copien el que ha decidit el host.
    void Apply(const MatchSummary& s) { _s = s; }

    const MatchSummary& Get() const { return _s; }

private:
    MatchSummary _s;
};

std::vector<uint8_t> EncodeMatchSummary(const MatchSummary& s);
bool DecodeMatchSummary(ByteReader& r, MatchSummary& out);
std::vector<uint8_t> EncodePickLevel(uint8_t levelId);
bool DecodePickLevel(ByteReader& r, uint8_t& levelId);

#endif