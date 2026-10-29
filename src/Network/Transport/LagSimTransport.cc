#include "LagSimTransport.hh"
#include <algorithm>

LagSimTransport::LagSimTransport(std::unique_ptr<ITransport> inner, LagSimSettings settings)
    : _inner(std::move(inner)), _settings(settings), _rng(std::random_device{}()) {}

bool LagSimTransport::StartHost(uint16_t port, int maxPeers) 
{
    return _inner->StartHost(port, maxPeers); 
}

bool LagSimTransport::Connect(const std::string& a, uint16_t p) 
{ 
    return _inner->Connect(a, p); 
}

void LagSimTransport::Disconnect(PeerId peer) 
{ 
    Pending p;
    p.release = std::max(Clock::now() + std::chrono::milliseconds(_settings.latencyMs),
                         _lastReliableRelease);
    _lastReliableRelease = p.release;
    p.peer = peer;
    p.channel = Channel::Reliable;
    p.disconnect = true;
    _pending.push_back(std::move(p));
}

int LagSimTransport::GetRttMs(PeerId peer) const 
{
    return _inner->GetRttMs(peer) + _settings.latencyMs + _settings.jitterMs / 2;
}

void LagSimTransport::Send(PeerId peer, Channel channel, const uint8_t* data, size_t size) 
{
    if (channel == Channel::Unreliable && _settings.lossChance > 0.0f) 
    {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        if (d(_rng) < _settings.lossChance) return; // Lost packet
    }

    int delay = _settings.latencyMs;
    if (_settings.jitterMs > 0) 
    {
        std::uniform_int_distribution<int> j(0, _settings.jitterMs);
        delay += j(_rng);
    }

    Pending p;
    p.release = Clock::now() + std::chrono::milliseconds(delay);
    p.peer = peer;
    p.channel = channel;
    p.data.assign(data, data + size);

    if (channel == Channel::Reliable && _settings.stallChance > 0.0f)
    {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        if (d(_rng) < _settings.stallChance) delay += _settings.stallMs;
    }

    if (channel == Channel::Reliable) 
    {
        // Never release a reliable packet before the last one, to avoid reordering.
        p.release = std::max(p.release, _lastReliableRelease);
        _lastReliableRelease = p.release;
    }

    if (channel == Channel::Unreliable && _settings.duplicateChance > 0.0f)
    {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        if (d(_rng) < _settings.duplicateChance)
        {
            Pending dup = _pending.back();
            std::uniform_int_distribution<int> extra(0, 40);
            dup.release += std::chrono::milliseconds(extra(_rng));
            _pending.push_back(std::move(dup));
        }
    }

    _pending.push_back(std::move(p));
}

void LagSimTransport::Poll(std::vector<NetEvent>& out) 
{
    const auto now = Clock::now();
    for (auto it = _pending.begin(); it != _pending.end();) 
    {
        if (it->release <= now) 
        {
            if (it->disconnect) _inner->Disconnect(it->peer);
            else                _inner->Send(it->peer, it->channel, it->data.data(), it->data.size());
            it = _pending.erase(it);
        } 
        else 
        {
            ++it;
        }
    }

    _inner->Poll(out);
}