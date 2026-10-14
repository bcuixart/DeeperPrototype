#ifndef DETERMINISTIC_RNG_HH
#define DETERMINISTIC_RNG_HH

#include <cstdint>
#include <utility>
#include <vector>

class DeterministicRng {
public:
    explicit DeterministicRng(uint32_t seed) : _state(seed ? seed : 0x9E3779B9u) {}

    uint32_t Next()   // Xorshift32
    {
        uint32_t x = _state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return _state = x;
    }

    uint32_t Range(uint32_t n) { return Next() % n; }

private:
    uint32_t _state;
};

template <typename T>
void DeterministicShuffle(std::vector<T>& v, DeterministicRng& rng)   // Fisher-Yates
{
    for (size_t i = v.size(); i > 1; --i)
        std::swap(v[i - 1], v[rng.Range((uint32_t)i)]);
}

#endif