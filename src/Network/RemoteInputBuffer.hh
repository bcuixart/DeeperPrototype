#ifndef REMOTE_INPUT_BUFFER_HH
#define REMOTE_INPUT_BUFFER_HH

#include "Network/NetworkTypes.hh"
#include <array>
#include <cstdint>

class RemoteInputBuffer
{
public:
    static constexpr uint32_t kDecayTicks = 30;   // ~0,5 s a 60 Hz sense dades: tornem a neutre

    void Reset();
    void Store(uint8_t slot, uint32_t tick, uint8_t input);

    // L'input del slot per a aquest tick. Si no el tenim, l'últim conegut de poc abans;
    // si fa massa que no sabem res d'aquest slot, retorna `neutral`. Mai espera.
    uint8_t Get(uint8_t slot, uint32_t tick, uint8_t neutral) const;

private:
    static constexpr int      kRingSize = 64;
    static constexpr uint32_t kLookback = 8;

    struct Entry { uint32_t tick = 0; uint8_t input = 0; bool valid = false; };
    struct SlotData {
        std::array<Entry, kRingSize> ring;
        uint32_t lastTick = 0;
        uint8_t lastInput = 0;
        bool any = false;
    };
    std::array<SlotData, kMaxSlots> _slots;
};

#endif