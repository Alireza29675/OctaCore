#include <assert.h>
#include <string.h>
#include "PaletteProtocol.h"

int main() {
    PaletteProtocol state;
    for (const auto& pixel : state.pixels) {
        for (uint8_t channel : pixel) assert(channel == 0);
    }
    assert(state.brightness == 100);
    uint8_t palette[49] = {2};
    for (int i = 0; i < 48; ++i) palette[i + 1] = i;
    assert(state.apply(palette, sizeof(palette)));
    uint8_t frame[31] = {3};
    memset(frame + 1, 0xf1, 30);
    assert(state.apply(frame, sizeof(frame)));
    for (int i = 0; i < LED_COUNT; ++i) {
        assert(state.pixels[i][0] == (i % 2 == 0 ? 3 : 45));
        assert(state.pixels[i][2] == (i % 2 == 0 ? 5 : 47));
    }
    uint8_t fill[] = {4, 15};
    assert(state.apply(fill, sizeof(fill)));
    assert(state.pixels[59][2] == 47);
    uint8_t brightness[] = {5, 255};
    assert(state.apply(brightness, sizeof(brightness)));
    assert(state.brightness == 255);

    const PaletteProtocol snapshot = state;
    for (int command = 0; command < 256; ++command) {
        uint8_t payload[64] = {};
        payload[0] = command;
        for (size_t length = 0; length < sizeof(payload); ++length) {
            PaletteProtocol candidate = snapshot;
            const bool expected = ((command == 1 || command == 4 || command == 5) && length == 2)
                || (command == 2 && length == 49) || (command == 3 && length == 31);
            assert(candidate.apply(payload, length) == expected);
            if (!expected || command == 1) assert(memcmp(&candidate, &snapshot, sizeof(snapshot)) == 0);
        }
    }
    for (int index = 16; index < 256; ++index) {
        uint8_t invalid[] = {4, static_cast<uint8_t>(index)};
        assert(!state.apply(invalid, sizeof(invalid)));
        assert(memcmp(&state, &snapshot, sizeof(state)) == 0);
    }
    assert(!state.apply(nullptr, 49));
}
