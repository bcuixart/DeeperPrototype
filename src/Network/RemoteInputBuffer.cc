#include "Network/RemoteInputBuffer.hh"

void RemoteInputBuffer::Reset() { _slots = {}; }

void RemoteInputBuffer::Store(uint8_t slot, uint32_t tick, uint8_t input)
{
    if (slot >= kMaxSlots) return;
    SlotData& d = _slots[slot];

    Entry& e = d.ring[tick % kRingSize];
    e.tick = tick;
    e.input = input;
    e.valid = true;

    if (!d.any || tick > d.lastTick)
    {
        d.lastTick = tick;
        d.lastInput = input;
        d.any = true;
    }
}

uint8_t RemoteInputBuffer::Get(uint8_t slot, uint32_t tick, uint8_t neutral, bool* exact) const
{
    if (exact) *exact = false;
    if (slot >= kMaxSlots) return neutral;
    const SlotData& d = _slots[slot];
    if (!d.any) return neutral;

    for (uint32_t k = 0; k <= kLookback && k <= tick; ++k)
    {
        const Entry& e = d.ring[(tick - k) % kRingSize];
        if (e.valid && e.tick == tick - k)
        {
            if (exact) *exact = (k == 0);
            return e.input;
        }
    }

    const uint32_t stale = (tick > d.lastTick) ? tick - d.lastTick : 0;
    return (stale > kDecayTicks) ? neutral : d.lastInput;
}