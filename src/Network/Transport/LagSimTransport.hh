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
};

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