#ifndef OCTACORE_CLUB_ENGINE_H
#define OCTACORE_CLUB_ENGINE_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "OctaCoreConfig.h"

static_assert(LED_COUNT == 60, "Club protocol v1 requires 60 LEDs per role.");

// Fixed-size protocol and renderer, independent of Arduino and networking.
class ClubEngine {
  public:
    struct Program {
        uint32_t revision = 0;
        uint16_t bpm100 = 12000;
        uint8_t effect = 0;
        uint8_t brightness = 64;
        uint8_t duty = 35;
        uint8_t division = 2;
        uint8_t dash = 4;
        uint8_t motion = 4;
        uint32_t seed = 0;
        uint8_t colors[4][3] = {};
        uint8_t steps[16] = {};
    };

    struct Reply {
        uint8_t bytes[9] = {};
        size_t length = 0;
    };

    // Unknown/malformed frames do not change program state or generate a reply.
    bool handle(const uint8_t* data, size_t length, uint32_t now, uint8_t role, Reply& reply) {
        reply.length = 0;
        if (data == nullptr || length == 0) return false;
        switch (data[0]) {
            case 32: {
                if (length != 46 || data[1] != 1) return false;
                Program candidate;
                candidate.revision = read32(data + 2);
                candidate.bpm100 = read16(data + 6);
                candidate.effect = data[8];
                candidate.brightness = data[9];
                candidate.duty = data[10];
                candidate.division = data[11];
                candidate.dash = data[12];
                candidate.motion = data[13];
                candidate.seed = read32(data + 14);
                if (candidate.bpm100 < 4000 || candidate.bpm100 > 24000
                    || candidate.effect > 5 || candidate.duty < 1 || candidate.duty > 100
                    || (candidate.division != 1 && candidate.division != 2 && candidate.division != 4)
                    || candidate.dash < 1 || candidate.dash > 15
                    || candidate.motion < 1 || candidate.motion > 16) return false;
                for (size_t i = 0; i < 16; ++i) {
                    if (data[30 + i] > 4) return false;
                }
                memcpy(candidate.colors, data + 18, sizeof(candidate.colors));
                memcpy(candidate.steps, data + 30, sizeof(candidate.steps));
                _staged = candidate;
                _hasStaged = true;
                acknowledge(reply, candidate.revision, 0);
                return true;
            }
            case 33: {
                if (length != 9 || !_hasStaged || read32(data + 1) != _staged.revision) return false;
                const uint32_t start = read32(data + 5);
                const int32_t delay = static_cast<int32_t>(start - now);
                if (delay < 0 || delay > 10000) return false;
                _pending = _staged;
                _pendingStart = start;
                _hasPending = true;
                acknowledge(reply, _pending.revision, 1);
                return true;
            }
            case 34:
                if (length != 5) return false;
                reply.bytes[0] = 35;
                memcpy(reply.bytes + 1, data + 1, 4);
                write32(reply.bytes + 5, now);
                reply.length = 9;
                return true;
            case 36:
                if (length != 1) return false;
                reply.bytes[0] = 37;
                reply.bytes[1] = 2;
                reply.bytes[2] = role;
                write32(reply.bytes + 3, activeRevision());
                reply.length = 7;
                return true;
            case 39:
                if (length != 1) return false;
                stop();
                return true;
            case 40: {
                if (length != 15) return false;
                const uint16_t bpm100 = read16(data + 5);
                const int32_t elapsed = static_cast<int32_t>(now - read32(data + 7));
                if (bpm100 < 4000 || bpm100 > 24000 || elapsed < -10000 || elapsed > 10000) return false;
                _tempoBpm100 = bpm100;
                _beatNumber = read32(data + 11);
                _elapsed = elapsed;
                _lastTick = now;
                _hasTempo = true;
                reply.bytes[0] = 41;
                memcpy(reply.bytes + 1, data + 1, 4);
                reply.length = 5;
                return true;
            }
            case 44:
                if (length != 6 || data[5] > 1) return false;
                _muted = data[5] != 0;
                memcpy(reply.bytes, data, 6);
                reply.bytes[0] = 45;
                reply.length = 6;
                return true;
            default:
                return false;
        }
    }

    void stop() {
        _running = false;
        _hasPending = false;
        _hasStaged = false;
        _muted = false;
    }

    bool running() const { return _running; }
    uint32_t activeRevision() const { return _running ? _active.revision : 0; }
    uint8_t brightness() const { return _active.brightness; }

    // Extend millis with integer deltas; render timing never accumulates frame error.
    void tick(uint32_t now) {
        if (_hasTempo) {
            _elapsed += static_cast<uint32_t>(now - _lastTick);
            _lastTick = now;
        }
        if (_hasPending && static_cast<int32_t>(now - _pendingStart) >= 0) {
            _active = _pending;
            _hasPending = false;
            _running = true;
            if (!_hasTempo) {
                _hasTempo = true;
                _tempoBpm100 = _active.bpm100;
                _beatNumber = 0;
                _elapsed = static_cast<uint32_t>(now - _pendingStart);
                _lastTick = now;
            }
        }
    }

    bool render(uint32_t now, uint8_t role, uint8_t pixels[LED_COUNT][3]) {
        tick(now);
        if (!_running) return false;
        memset(pixels, 0, LED_COUNT * 3);
        if (_muted) return true;
        const double beat = _beatNumber + static_cast<double>(_elapsed) * (_tempoBpm100 / 100.0) / 60000.0;
        if (beat < 0) return true;
        const double position = beat * _active.division;
        const uint64_t step = static_cast<uint64_t>(floor(position));
        const double fraction = position - floor(position);
        const double duty = _active.duty / 100.0;
        const uint8_t cell = _active.steps[step % 16];
        if (cell == 0 || fraction >= duty) return true;
        const uint8_t color = cell == 4
            ? hash32(_active.seed ^ static_cast<uint32_t>(step)) % 4 : cell - 1;
        const double tail = 1.0 - fraction / duty;
        const double gain = _active.effect == 5 ? tail * tail : 1.0;
        for (size_t i = 0; i < LED_COUNT; ++i) {
            // Towers 3/4 repeat the established 120-pixel choreography of 1/2.
            const uint32_t g = (role % 2) * LED_COUNT + i;
            bool lit = true;
            switch (_active.effect) {
                case 1:
                    lit = fmod(g + floor(beat * _active.motion * 4), _active.dash * 2) < _active.dash;
                    break;
                case 2: {
                    const double phase = fmod(beat * _active.motion / 4, 2.0);
                    const double head = (phase <= 1.0 ? phase : 2.0 - phase) * 119;
                    lit = fabs(g - head) < _active.dash;
                    break;
                }
                case 3:
                    lit = fmod(g + floor(beat * _active.motion * 8), 120) < _active.dash * 3;
                    break;
                case 4:
                    lit = hash32(_active.seed ^ static_cast<uint32_t>(step)
                        ^ ((g + 1) * UINT32_C(0x9e3779b9))) % 16 < _active.dash;
                    break;
            }
            if (lit) {
                for (size_t channel = 0; channel < 3; ++channel) {
                    pixels[i][channel] = static_cast<uint8_t>(floor(_active.colors[color][channel] * gain));
                }
            }
        }
        return true;
    }

    static uint32_t hash32(uint32_t value) {
        value ^= value >> 16;
        value *= UINT32_C(0x7feb352d);
        value ^= value >> 15;
        value *= UINT32_C(0x846ca68b);
        return value ^ (value >> 16);
    }

    static uint16_t read16(const uint8_t* data) {
        return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
    }

    static uint32_t read32(const uint8_t* data) {
        return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8)
            | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
    }

    static void write32(uint8_t* data, uint32_t value) {
        for (size_t i = 0; i < 4; ++i) data[i] = static_cast<uint8_t>(value >> (i * 8));
    }

  private:
    static void acknowledge(Reply& reply, uint32_t revision, uint8_t status) {
        reply.bytes[0] = 38;
        write32(reply.bytes + 1, revision);
        reply.bytes[5] = status;
        reply.length = 6;
    }

    Program _active;
    Program _staged;
    Program _pending;
    bool _running = false;
    bool _hasStaged = false;
    bool _hasPending = false;
    bool _hasTempo = false;
    bool _muted = false;
    uint16_t _tempoBpm100 = 12000;
    uint32_t _beatNumber = 0;
    uint32_t _pendingStart = 0;
    uint32_t _lastTick = 0;
    int64_t _elapsed = 0;
};

#endif
