#ifndef ROUNDMANAGER_HH
#define ROUNDMANAGER_HH

#include <array>
#include <cstdint>
#include <vector>
#include "Network/NetworkTypes.hh"
#include "Network/Protocol/ByteStream.hh"

// Tipus d'esdeveniment de ronda (el NetworkManager no els coneix).
enum class RoundEvent : uint8_t { ScoreChanged = 1, RoundEnd = 2 };

class RoundManager {
public:
    static constexpr uint32_t kDefaultDurationTicks = 1200;   // 20 s per provar; després 3600 (1 minut)

    void Begin(uint32_t durationTicks, bool authority, const std::array<bool, kMaxSlots>& active);

    bool IsAuthority() const { return _authority; }
    bool IsOver(uint32_t levelTick) const { return levelTick >= _endTick; }
    uint32_t EndTick() const { return _endTick; }
    uint32_t TicksLeft(uint32_t levelTick) const { return levelTick >= _endTick ? 0 : _endTick - levelTick; }
    bool IsActive(uint8_t slot) const { return slot < kMaxSlots && _active[slot]; }
    uint16_t GetScore(uint8_t slot) const { return slot < kMaxSlots ? _score[slot] : 0; }
    const std::array<uint16_t, kMaxSlots>& GetScores() const { return _score; }

    // Només autoritat (host / offline)
    bool AddScore(uint8_t slot, int points, uint16_t& newScore);
    void EndEarly(uint32_t levelTick) { if (_authority && levelTick < _endTick) _endTick = levelTick; }
    bool TakeEndAnnounce();   // true una sola vegada, quan s'ha de comunicar el final

    // Només client (el host mana)
    void SetScore(uint8_t slot, uint16_t score) { if (slot < kMaxSlots) _score[slot] = score; }
    void ApplyEnd(uint32_t endTick, const std::array<uint16_t, kMaxSlots>& scores) { _endTick = endTick; _score = scores; }

private:
    bool _authority = true;
    uint32_t _endTick = 0;
    bool _endAnnounced = false;
    std::array<uint16_t, kMaxSlots> _score{};
    std::array<bool, kMaxSlots> _active{};
};

std::vector<uint8_t> EncodeScoreChanged(uint8_t slot, uint16_t score);
bool DecodeScoreChanged(ByteReader& r, uint8_t& slot, uint16_t& score);
std::vector<uint8_t> EncodeRoundEnd(uint32_t endTick, const std::array<uint16_t, kMaxSlots>& scores);
bool DecodeRoundEnd(ByteReader& r, uint32_t& endTick, std::array<uint16_t, kMaxSlots>& scores);

#endif