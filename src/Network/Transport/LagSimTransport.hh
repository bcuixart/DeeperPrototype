#ifndef LAGSIMTRANSPORT_HH
#define LAGSIMTRANSPORT_HH

#include "ITransport.hh"
#include <chrono>
#include <deque>
#include <memory>
#include <random>

struct LagSimSettings {
    int   latencyMs  = 0;
    int   jitterMs   = 0;
    float lossChance = 0.0f;
    float duplicateChance = 0.0f;
    float stallChance     = 0.0f;
    int   stallMs         = 200;
};

inline bool LagProfileFromName(char name, LagSimSettings& out)
{
    out = LagSimSettings{};
    switch (std::toupper((unsigned char)name))
    {
    case 'A': out.latencyMs = 25;  out.jitterMs = 5;   out.lossChance = 0.0f;  out.duplicateChance = 0.0f;  out.stallChance = 0.0f;  out.stallMs = 0;   return true;
    case 'B': out.latencyMs = 60;  out.jitterMs = 20;  out.lossChance = 0.02f; out.duplicateChance = 0.01f; out.stallChance = 0.0f;  out.stallMs = 0;   return true;
    case 'C': out.latencyMs = 120; out.jitterMs = 40;  out.lossChance = 0.08f; out.duplicateChance = 0.03f; out.stallChance = 0.02f; out.stallMs = 250; return true;
    case 'D': out.latencyMs = 200; out.jitterMs = 100; out.lossChance = 0.15f; out.duplicateChance = 0.05f; out.stallChance = 0.05f; out.stallMs = 400; return true;
    default:  return false;
    }
}

class LagSimTransport : public ITransport {
public:
    LagSimTransport(std::unique_ptr<ITransport> inner, LagSimSettings settings);

    bool StartHost(uint16_t port, int maxPeers) override;
    bool Connect(const std::string& address, uint16_t port) override;
    void Disconnect(PeerId peer) override;

    void Send(PeerId peer, Channel channel, const uint8_t* data, size_t size) override;

    void Poll(std::vector<NetEvent>& out) override;

    int  GetRttMs(PeerId peer) const override;

    void SetSettings(const LagSimSettings& s) { _settings = s; }

private:
    using Clock = std::chrono::steady_clock;

    struct Pending {
        Clock::time_point release;
        PeerId peer;
        Channel channel;
        bool disconnect = false;
        std::vector<uint8_t> data;
    };

    std::unique_ptr<ITransport> _inner;
    LagSimSettings _settings;
    std::deque<Pending> _pending;
    Clock::time_point _lastReliableRelease{};
    std::mt19937 _rng;
};

#endif