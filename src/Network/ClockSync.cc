#include "Network/ClockSync.hh"
#include <algorithm>
#include <cmath>

void ClockSync::Reset()
{
    _error = 0.0f;
    _hasSample = false;
}

void ClockSync::OnSample(uint32_t hostTick, uint32_t localTick, int rttMs, float tickSeconds)
{
    const float rttTicks = (rttMs / 1000.0f) / tickSeconds;
    const float target = (float)hostTick + rttTicks + kCushionTicks;
    const float e = target - (float)localTick;

    _error = _hasSample ? 0.5f * _error + 0.5f * e : e;   // suavitzat lleuger
    _hasSample = true;
}

float ClockSync::GetTimeScale() const
{
    if (!_hasSample || std::fabs(_error) <= kDeadZoneTicks) return 1.0f;
    return std::clamp(1.0f + _error * kGain, kMinScale, kMaxScale);
}