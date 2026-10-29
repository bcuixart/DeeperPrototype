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
    float GetCushionTicks() const { return _cushion; }

private:
    static constexpr float kCushionBase = 0.5f;        // ticks de coixí amb xarxa perfecta
    static constexpr float kCushionPerRttTick = 0.2f;  // ticks extra per cada tick de RTT
    static constexpr float kCushionMax = 5.0f;
    static constexpr float kDeadZoneTicks = 0.5f; // sota això no toquem el ritme
    static constexpr float kGain = 0.04f;          // ajust de velocitat per tick d'error
    static constexpr float kMinScale = 0.9f;
    static constexpr float kMaxScale = 1.2f;
    static constexpr float kFarTicks = 20.0f;       // a partir d'aquí corregim més fort
    static constexpr float kVeryFarTicks = 60.0f;   // i a partir d'aquí, encara més
    static constexpr int   kMaxSaneRttMs = 1000;    // una mostra més lenta que això va creuar una congelació

    float _error = 0.0f;       // tick objectiu - tick actual (positiu = anem endarrerits)
    float _cushion = kCushionBase;
    bool _hasSample = false;
};

#endif