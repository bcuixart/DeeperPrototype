#ifndef CLOCK_SYNC_HH
#define CLOCK_SYNC_HH

#include <cstdint>

// Calcula quant ha d'anar ràpid o lent el rellotge del client perquè quedi
// lleugerament per davant del host. La simulació segueix sent de pas fix:
// això només canvia amb quina freqüència s'executen els passos.
class ClockSync
{
public:
    void Reset();
    void OnSample(uint32_t hostTick, uint32_t localTick, int rttMs, float tickSeconds);

    // 1.0 = velocitat normal. >1 el client va tard i ha d'accelerar, <1 va massa avançat.
    float GetTimeScale() const;
    float GetErrorTicks() const { return _error; }

private:
    static constexpr float kCushionTicks = 2.0f;   // marge per a jitter i per al retard d'un frame
    static constexpr float kDeadZoneTicks = 0.75f; // sota això no toquem el ritme
    static constexpr float kGain = 0.03f;          // ajust de velocitat per tick d'error
    static constexpr float kMinScale = 0.9f;
    static constexpr float kMaxScale = 1.2f;

    float _error = 0.0f;       // tick objectiu - tick actual (positiu = anem endarrerits)
    bool _hasSample = false;
};

#endif