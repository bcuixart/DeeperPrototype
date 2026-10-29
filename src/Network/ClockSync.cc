#include "Network/ClockSync.hh"
#include <algorithm>
#include <cmath>

void ClockSync::Reset()
{
    _error = 0.0f;
    _cushion = kCushionBase;
    _hasSample = false;
}

void ClockSync::OnSample(uint32_t hostTick, uint32_t localTick, int rttMs, float tickSeconds)
{
    if (rttMs > kMaxSaneRttMs) return;   // mostra enverinada per una congelació: la ignorem

    const float rttTicks = (rttMs / 1000.0f) / tickSeconds;
    _cushion = std::clamp(kCushionBase + kCushionPerRttTick * rttTicks, kCushionBase, kCushionMax);
    const float target = (float)hostTick + rttTicks + _cushion;
    const float e = target - (float)localTick;

    _error = _hasSample ? 0.5f * _error + 0.5f * e : e;   // suavitzat lleuger
    _hasSample = true;
}

float ClockSync::GetTimeScale() const
{
    if (!_hasSample) return 1.0f;

    const float a = std::fabs(_error);
    if (a <= kDeadZoneTicks) return 1.0f;

    // Errors molt grans (congelació del host o d'un client): correcció forta fins a tornar al marge normal.
    // Mai arribem a 0: si el rellotge s'aturés no s'enviarien pings i no tornaríem a rebre mostres.
    if (a > kVeryFarTicks) return _error < 0 ? 0.4f : 1.6f;
    if (a > kFarTicks)     return _error < 0 ? 0.7f : 1.3f;

    return std::clamp(1.0f + _error * kGain, kMinScale, kMaxScale);
}