#include "Level/RoundManager.hh"

void RoundManager::Begin(uint32_t durationTicks, bool authority, const std::array<bool, kMaxSlots>& active)
{
    _authority = authority;
    _endTick = durationTicks;
    _endAnnounced = false;
    _score = {};
    _active = active;
}

bool RoundManager::AddScore(uint8_t slot, int points, uint16_t& newScore)
{
    if (!_authority || slot >= kMaxSlots || points == 0) return false;
    int s = static_cast<int>(_score[slot]) + points;
    if (s < 0) s = 0;
    if (s > 65535) s = 65535;
    _score[slot] = static_cast<uint16_t>(s);
    newScore = _score[slot];
    return true;
}

bool RoundManager::TakeEndAnnounce()
{
    if (_endAnnounced) return false;
    _endAnnounced = true;
    return true;
}

// Les puntuacions viatgen com a valor ABSOLUT: si un missatge es repeteix, no passa res.
std::vector<uint8_t> EncodeScoreChanged(uint8_t slot, uint16_t score)
{
    ByteWriter w;
    w.U8(slot);
    w.U16(score);
    return w.Data();
}

bool DecodeScoreChanged(ByteReader& r, uint8_t& slot, uint16_t& score)
{
    slot = r.U8();
    score = r.U16();
    return r.Ok() && slot < kMaxSlots;
}

std::vector<uint8_t> EncodeRoundEnd(uint32_t endTick, const std::array<uint16_t, kMaxSlots>& scores)
{
    ByteWriter w;
    w.U32(endTick);
    for (uint16_t s : scores) w.U16(s);
    return w.Data();
}

bool DecodeRoundEnd(ByteReader& r, uint32_t& endTick, std::array<uint16_t, kMaxSlots>& scores)
{
    endTick = r.U32();
    for (uint16_t& s : scores) s = r.U16();
    return r.Ok();
}

std::vector<uint8_t> EncodePlayerLeft(uint8_t slot)
{
    ByteWriter w;
    w.U8(slot);
    return w.Data();
}

bool DecodePlayerLeft(ByteReader& r, uint8_t& slot)
{
    slot = r.U8();
    return r.Ok() && slot < kMaxSlots;
}