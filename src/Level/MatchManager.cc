#include "Level/MatchManager.hh"
#include <algorithm>
#include <random>

// Entre els candidats guanya qui té més 'primary'; si hi ha empat, més 'secondary'; si encara
// hi ha empat, el host tria a l'atzar (i ho difon, així tothom veu el mateix).
static uint8_t PickWinner(const std::array<uint16_t, kMaxSlots>& primary,
                          const std::array<uint16_t, kMaxSlots>& secondary,
                          uint8_t candidates)
{
    int bestP = -1, bestS = -1;
    std::vector<uint8_t> best;
    for (uint8_t s = 0; s < kMaxSlots; ++s)
    {
        if (!(candidates & (1 << s))) continue;
        const int p = primary[s], q = secondary[s];
        if (p > bestP || (p == bestP && q > bestS)) { bestP = p; bestS = q; best.clear(); }
        if (p == bestP && q == bestS) best.push_back(s);
    }
    if (best.empty()) return kNoWinner;

    static std::mt19937 rng{ std::random_device{}() };
    return best[rng() % best.size()];
}

void MatchManager::Reset(uint8_t roundCount)
{
    _s = MatchSummary{};
    _s.roundCount = roundCount < 1 ? 1 : roundCount;
}

void MatchManager::SetRound(uint8_t index, uint8_t count)
{
    _s.roundIndex = index;
    _s.roundCount = count;
    _s.isFinal = false;
}

void MatchManager::EndRound(const std::array<uint16_t, kMaxSlots>& scores,
                            const std::array<bool, kMaxSlots>& active,
                            const std::array<bool, kMaxSlots>& gone)
{
    uint8_t candidates = 0;
    for (uint8_t s = 0; s < kMaxSlots; ++s)
    {
        _s.lastRound[s] = active[s] ? scores[s] : 0;
        if (!active[s]) continue;

        _s.playedMask |= (1 << s);
        _s.totals[s] = (uint16_t)std::min<int>(_s.totals[s] + _s.lastRound[s], 65535);
        if (!gone[s]) candidates |= (1 << s);
    }
    _s.presentMask = candidates;

    _s.roundWinner = PickWinner(_s.lastRound, _s.totals, candidates);
    _s.isFinal = _s.roundIndex + 1 >= _s.roundCount;
    _s.matchWinner = _s.isFinal ? PickWinner(_s.totals, _s.lastRound, candidates) : kNoWinner;
}

std::vector<uint8_t> EncodeMatchSummary(const MatchSummary& s)
{
    ByteWriter w;
    w.U8(s.roundIndex);
    w.U8(s.roundCount);
    w.U8(s.roundWinner);
    w.U8(s.matchWinner);
    w.U8(s.isFinal ? 1 : 0);
    w.U8(s.playedMask);
    w.U8(s.presentMask);
    for (uint16_t v : s.lastRound) w.U16(v);
    for (uint16_t v : s.totals) w.U16(v);
    return w.Data();
}

bool DecodeMatchSummary(ByteReader& r, MatchSummary& out)
{
    out.roundIndex = r.U8();
    out.roundCount = r.U8();
    out.roundWinner = r.U8();
    out.matchWinner = r.U8();
    out.isFinal = r.U8() != 0;
    out.playedMask = r.U8();
    out.presentMask = r.U8();
    for (uint16_t& v : out.lastRound) v = r.U16();
    for (uint16_t& v : out.totals) v = r.U16();

    if (!r.Ok() || out.roundCount == 0 || out.roundIndex >= out.roundCount) return false;
    if (out.roundWinner != kNoWinner && out.roundWinner >= kMaxSlots) return false;
    if (out.matchWinner != kNoWinner && out.matchWinner >= kMaxSlots) return false;
    return true;
}

std::vector<uint8_t> EncodePickLevel(uint8_t slot, uint8_t levelId)
{
    ByteWriter w;
    w.U8(slot);
    w.U8(levelId);
    return w.Data();
}

bool DecodePickLevel(ByteReader& r, uint8_t& slot, uint8_t& levelId)
{
    slot = r.U8();
    levelId = r.U8();
    return r.Ok() && slot < kMaxSlots;
}