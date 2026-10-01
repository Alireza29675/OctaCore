#include <assert.h>
#include <string.h>
#include "ClubEngine.h"

static void stageFrame(uint8_t (&frame)[46], uint32_t revision = 7, uint8_t effect = 0) {
    memset(frame, 0, sizeof(frame));
    frame[0] = 32;
    frame[1] = 1;
    ClubEngine::write32(frame + 2, revision);
    frame[6] = 12000 & 255;
    frame[7] = 12000 >> 8;
    frame[8] = effect;
    frame[9] = 64;
    frame[10] = 100;
    frame[11] = 2;
    frame[12] = 4;
    frame[13] = 4;
    ClubEngine::write32(frame + 14, 12345);
    const uint8_t colors[] = {100, 80, 40, 10, 20, 30, 50, 60, 70, 90, 100, 110};
    memcpy(frame + 18, colors, sizeof(colors));
    memset(frame + 30, 1, 16);
}

static void commitFrame(uint8_t (&frame)[9], uint32_t revision, uint32_t start) {
    frame[0] = 33;
    ClubEngine::write32(frame + 1, revision);
    ClubEngine::write32(frame + 5, start);
}

static void start(ClubEngine& engine, uint8_t effect = 0, uint32_t at = 0) {
    uint8_t stage[46];
    stageFrame(stage, 7, effect);
    ClubEngine::Reply reply;
    assert(engine.handle(stage, sizeof(stage), at, 0, reply));
    assert(reply.length == 6 && reply.bytes[0] == 38 && reply.bytes[5] == 0);
    assert(ClubEngine::read32(reply.bytes + 1) == 7);
    uint8_t commit[9];
    commitFrame(commit, 7, at);
    assert(engine.handle(commit, sizeof(commit), at, 0, reply));
    assert(reply.length == 6 && reply.bytes[5] == 1);
}

static void expectRejected(ClubEngine& engine, const uint8_t* frame, size_t length) {
    uint8_t before[sizeof(engine)];
    memcpy(before, &engine, sizeof(engine));
    ClubEngine::Reply reply;
    assert(!engine.handle(frame, length, 100, 0, reply));
    assert(reply.length == 0);
    assert(memcmp(before, &engine, sizeof(engine)) == 0);
}

int main() {
    ClubEngine engine;
    uint8_t pixels[LED_COUNT][3] = {};
    assert(!engine.render(0, 0, pixels));
    assert(!engine.running() && engine.activeRevision() == 0);
    assert(ClubEngine::hash32(0) == 0);
    assert(ClubEngine::hash32(1) == UINT32_C(1753845952));

    uint8_t stage[46];
    stageFrame(stage);
    ClubEngine::Reply reply;
    assert(engine.handle(stage, sizeof(stage), 100, 0, reply));
    for (size_t length = 0; length < sizeof(stage); ++length) expectRejected(engine, stage, length);
    expectRejected(engine, nullptr, 46);
    const uint8_t badFields[][2] = {
        {1, 2}, {7, 0}, {7, 255}, {8, 6}, {10, 0}, {10, 101},
        {11, 0}, {11, 3}, {12, 0}, {12, 16}, {13, 0}, {13, 17}, {45, 5}
    };
    for (const auto& field : badFields) {
        uint8_t invalid[46];
        memcpy(invalid, stage, sizeof(stage));
        invalid[field[0]] = field[1];
        expectRejected(engine, invalid, sizeof(invalid));
    }
    for (int command = 0; command < 256; ++command) {
        uint8_t frame[64] = {};
        frame[0] = command;
        for (size_t length = 0; length < sizeof(frame); ++length) {
            if ((command == 34 && length == 5) || (command == 36 && length == 1)
                || (command == 39 && length == 1) || (command == 44 && length == 6)) continue;
            expectRejected(engine, frame, length);
        }
    }

    uint8_t commit[9];
    commitFrame(commit, 8, 850);
    expectRejected(engine, commit, sizeof(commit)); // wrong revision
    commitFrame(commit, 7, 99);
    expectRejected(engine, commit, sizeof(commit)); // past
    commitFrame(commit, 7, 10101);
    expectRejected(engine, commit, sizeof(commit)); // beyond ten seconds
    commitFrame(commit, 7, 850);
    assert(engine.handle(commit, sizeof(commit), 100, 0, reply));
    assert(!engine.render(849, 0, pixels));
    assert(engine.render(850, 0, pixels));
    assert(engine.activeRevision() == 7 && pixels[0][0] == 100 && engine.brightness() == 64);

    stageFrame(stage, 8, 1);
    stage[18] = 200;
    assert(engine.handle(stage, sizeof(stage), 900, 0, reply));
    assert(engine.render(900, 0, pixels) && pixels[0][0] == 100); // stage does not activate
    commitFrame(commit, 8, 1100);
    assert(engine.handle(commit, sizeof(commit), 900, 0, reply));
    stageFrame(stage, 9, 0);
    assert(engine.handle(stage, sizeof(stage), 950, 0, reply)); // cannot mutate pending revision
    assert(engine.render(1099, 0, pixels) && engine.activeRevision() == 7);
    assert(engine.render(1100, 0, pixels) && engine.activeRevision() == 8 && pixels[0][0] == 200);

    const uint8_t hello[] = {36};
    assert(engine.handle(hello, sizeof(hello), 1100, 1, reply));
    assert(reply.length == 7 && reply.bytes[0] == 37 && reply.bytes[1] == 2 && reply.bytes[2] == 1);
    assert(ClubEngine::read32(reply.bytes + 3) == 8);
    const uint8_t clock[] = {34, 0xde, 0xad, 0xbe, 0xef};
    assert(engine.handle(clock, sizeof(clock), UINT32_C(0xfffffff0), 0, reply));
    assert(reply.length == 9 && reply.bytes[0] == 35 && memcmp(reply.bytes + 1, clock + 1, 4) == 0);
    assert(ClubEngine::read32(reply.bytes + 5) == UINT32_C(0xfffffff0));
    const uint8_t stop[] = {39};
    assert(engine.handle(stop, sizeof(stop), 1101, 0, reply));
    assert(!engine.render(2000, 0, pixels) && engine.activeRevision() == 0);
    commitFrame(commit, 9, 2100);
    assert(!engine.handle(commit, sizeof(commit), 2000, 0, reply));

    ClubEngine wrapping;
    stageFrame(stage);
    assert(wrapping.handle(stage, sizeof(stage), UINT32_C(0xfffffff0), 0, reply));
    commitFrame(commit, 7, 24);
    assert(wrapping.handle(commit, sizeof(commit), UINT32_C(0xfffffff0), 0, reply));
    assert(!wrapping.render(23, 0, pixels));
    assert(wrapping.render(24, 0, pixels) && pixels[0][0] == 100);

    // Stop cancels an acknowledged future start, including through rollover.
    ClubEngine stoppedPending;
    stageFrame(stage);
    assert(stoppedPending.handle(stage, sizeof(stage), 900, 0, reply));
    commitFrame(commit, 7, 1000);
    assert(stoppedPending.handle(commit, sizeof(commit), 900, 0, reply));
    assert(stoppedPending.handle(stop, sizeof(stop), 900, 0, reply));
    assert(!stoppedPending.render(1000, 0, pixels));

    // The renderer extends uptime beyond one full millis period without resetting phase.
    ClubEngine longRunning;
    stageFrame(stage, 7, 0);
    memset(stage + 30, 4, 16);
    assert(longRunning.handle(stage, sizeof(stage), 0, 0, reply));
    commitFrame(commit, 7, 0);
    assert(longRunning.handle(commit, sizeof(commit), 0, 0, reply));
    assert(longRunning.render(0, 0, pixels));
    assert(longRunning.render(UINT32_C(0xfffffff0), 0, pixels));
    assert(longRunning.render(100, 0, pixels));
    const uint64_t extended = UINT64_C(0x100000000) + 100;
    const uint32_t longStep = static_cast<uint32_t>(extended / 250);
    const uint8_t longColor = ClubEngine::hash32(12345 ^ longStep) % 4;
    assert(memcmp(pixels[0], stage + 18 + longColor * 3, 3) == 0);

    ClubEngine pulse;
    start(pulse, 5);
    assert(pulse.render(0, 0, pixels) && pixels[0][0] == 100);
    assert(pulse.render(125, 0, pixels) && pixels[0][0] == 25 && pixels[0][1] == 20);
    assert(pulse.render(250, 0, pixels) && pixels[0][0] == 100);

    ClubEngine gate;
    stageFrame(stage);
    stage[10] = 50;
    stage[31] = 0;
    assert(gate.handle(stage, sizeof(stage), 0, 0, reply));
    commitFrame(commit, 7, 0);
    assert(gate.handle(commit, sizeof(commit), 0, 0, reply));
    assert(gate.render(124, 0, pixels) && pixels[0][0] == 100);
    assert(gate.render(125, 0, pixels) && pixels[0][0] == 0);
    assert(gate.render(250, 0, pixels) && pixels[0][0] == 0);
    assert(gate.render(500, 0, pixels) && pixels[0][0] == 100);

    ClubEngine dash;
    start(dash, 1);
    assert(dash.render(0, 0, pixels) && pixels[0][0] == 100 && pixels[4][0] == 0);
    assert(dash.render(0, 1, pixels) && pixels[0][0] == 0 && pixels[4][0] == 100);
    ClubEngine bounce;
    start(bounce, 2);
    assert(bounce.render(0, 0, pixels) && pixels[0][0] == 100 && pixels[4][0] == 0);
    assert(bounce.render(500, 1, pixels) && pixels[59][0] == 100 && pixels[55][0] == 0);
    ClubEngine orbit;
    start(orbit, 3);
    assert(orbit.render(0, 0, pixels) && pixels[11][0] == 100 && pixels[12][0] == 0);
    assert(orbit.render(500, 1, pixels) && pixels[28][0] == 100 && pixels[27][0] == 0);

    ClubEngine sparksA, sparksB;
    start(sparksA, 4);
    start(sparksB, 4);
    uint8_t other[LED_COUNT][3];
    for (uint32_t now = 0; now < 10000; now += 37) {
        sparksA.render(now, 1, pixels);
        sparksB.render(now, 1, other);
        assert(memcmp(pixels, other, sizeof(pixels)) == 0);
    }
    // Frame skipping must not change phase or output.
    sparksA.render(123456, 1, pixels);
    sparksB.render(123456, 1, other);
    assert(memcmp(pixels, other, sizeof(pixels)) == 0);

    // TEMPO establishes a clock without starting or staging an effect.
    ClubEngine independent;
    uint8_t tempo[15] = {40};
    ClubEngine::write32(tempo + 1, 42);
    tempo[5] = 12000 & 255;
    tempo[6] = 12000 >> 8;
    ClubEngine::write32(tempo + 7, 1000);
    ClubEngine::write32(tempo + 11, 17);
    assert(independent.handle(tempo, sizeof(tempo), 1100, 0, reply));
    assert(reply.length == 5 && reply.bytes[0] == 41 && ClubEngine::read32(reply.bytes + 1) == 42);
    assert(!independent.render(1100, 0, pixels));
    commitFrame(commit, 7, 1200);
    assert(!independent.handle(commit, sizeof(commit), 1100, 0, reply));
    stageFrame(stage, 7, 5);
    stage[6] = 24000 & 255; // Program BPM cannot overwrite the clock.
    stage[7] = 24000 >> 8;
    assert(independent.handle(stage, sizeof(stage), 1100, 0, reply));
    assert(independent.handle(commit, sizeof(commit), 1100, 0, reply));
    assert(independent.render(1125, 0, pixels) == false);
    assert(independent.render(1200, 0, pixels));
    assert(pixels[0][0] >= 3 && pixels[0][0] <= 4); // beat17.4, fraction.8, gain.04.
    assert(independent.handle(stop, sizeof(stop), 1201, 0, reply));
    independent.tick(1250);
    commitFrame(commit, 7, 1375);
    assert(independent.handle(stage, sizeof(stage), 1300, 0, reply));
    assert(independent.handle(commit, sizeof(commit), 1300, 0, reply));
    assert(independent.render(1375, 0, pixels) && pixels[0][0] == 25);
    // A delayed clock update changes phase only, retaining the active revision.
    ClubEngine::write32(tempo + 7, 1300);
    ClubEngine::write32(tempo + 11, 30);
    assert(independent.handle(tempo, sizeof(tempo), 1425, 0, reply));
    assert(independent.render(1425, 0, pixels) && pixels[0][0] == 25);
    assert(independent.activeRevision() == 7);
    for (size_t length = 0; length < sizeof(tempo); ++length) expectRejected(independent, tempo, length);
    uint8_t invalidTempo[16];
    memcpy(invalidTempo, tempo, sizeof(tempo));
    expectRejected(independent, invalidTempo, sizeof(invalidTempo));
    invalidTempo[5] = invalidTempo[6] = 0;
    expectRejected(independent, invalidTempo, 15);
    memcpy(invalidTempo, tempo, sizeof(tempo));
    ClubEngine::write32(invalidTempo + 7, 10101); // expectRejected uses now100.
    expectRejected(independent, invalidTempo, 15);
    ClubEngine::write32(invalidTempo + 7, static_cast<uint32_t>(100 - 10001));
    expectRejected(independent, invalidTempo, 15);

    // A future beat-zero anchor remains black until the beat reaches zero.
    ClubEngine futureClock;
    ClubEngine::write32(tempo + 7, 1000);
    ClubEngine::write32(tempo + 11, 0);
    assert(futureClock.handle(tempo, sizeof(tempo), 900, 0, reply));
    stageFrame(stage);
    assert(futureClock.handle(stage, sizeof(stage), 900, 0, reply));
    commitFrame(commit, 7, 900);
    assert(futureClock.handle(commit, sizeof(commit), 900, 0, reply));
    assert(futureClock.render(999, 0, pixels) && pixels[0][0] == 0);
    assert(futureClock.render(1000, 0, pixels) && pixels[0][0] == 100);

    // Clock anchor validation and signed elapsed survive millis rollover.
    ClubEngine wrapClock;
    ClubEngine::write32(tempo + 7, UINT32_C(0xfffffff0));
    ClubEngine::write32(tempo + 11, 16);
    assert(wrapClock.handle(tempo, sizeof(tempo), 24, 0, reply));
    stageFrame(stage, 7, 5);
    assert(wrapClock.handle(stage, sizeof(stage), 24, 0, reply));
    commitFrame(commit, 7, 24);
    assert(wrapClock.handle(commit, sizeof(commit), 24, 0, reply));
    assert(wrapClock.render(109, 0, pixels) && pixels[0][0] == 25);

    ClubEngine held;
    uint8_t blackout[6] = {44, 123, 0, 0, 0, 1};
    assert(held.handle(blackout, sizeof(blackout), 0, 0, reply));
    assert(reply.length == 6 && reply.bytes[0] == 45);
    assert(memcmp(reply.bytes + 1, blackout + 1, 5) == 0);
    // Hold before staging/commit survives activation and keeps clock advancing.
    stageFrame(stage, 7, 5);
    assert(held.handle(stage, sizeof(stage), 0, 0, reply));
    commitFrame(commit, 7, 1000);
    assert(held.handle(commit, sizeof(commit), 0, 0, reply));
    assert(held.render(1000, 0, pixels) && pixels[0][0] == 0);
    assert(held.render(1124, 0, pixels) && pixels[0][0] == 0);
    assert(held.running() && held.activeRevision() == 7);
    blackout[1] = 124;
    blackout[5] = 0;
    assert(held.handle(blackout, sizeof(blackout), 1125, 0, reply));
    assert(held.render(1125, 0, pixels) && pixels[0][0] == 25);
    for (size_t length = 0; length < sizeof(blackout); ++length) expectRejected(held, blackout, length);
    uint8_t malformedHold[7] = {44, 0, 0, 0, 0, 2, 0};
    expectRejected(held, malformedHold, 6);
    expectRejected(held, malformedHold, 7);
    blackout[5] = 1;
    assert(held.handle(blackout, sizeof(blackout), 1150, 0, reply));
    assert(held.handle(stop, sizeof(stop), 1151, 0, reply));
    blackout[5] = 0;
    assert(held.handle(blackout, sizeof(blackout), 1152, 0, reply));
    assert(!held.render(1200, 0, pixels) && !held.running());
    // Permanent STOP clears the latch even if a separate release is lost.
    blackout[5] = 1;
    assert(held.handle(blackout, sizeof(blackout), 1200, 0, reply));
    assert(held.handle(stop, sizeof(stop), 1201, 0, reply));
    stageFrame(stage);
    commitFrame(commit, 7, 1250);
    assert(held.handle(stage, sizeof(stage), 1202, 0, reply));
    assert(held.handle(commit, sizeof(commit), 1202, 0, reply));
    assert(held.render(1250, 0, pixels) && pixels[0][0] == 100);

    // Wire identities remain distinct while visual choreography repeats in pairs.
    for (uint8_t role = 0; role < 4; ++role) {
        ClubEngine tower;
        assert(tower.handle(hello, sizeof(hello), 0, role, reply));
        assert(reply.bytes[2] == role);
        for (uint8_t effect = 0; effect < 6; ++effect) {
            ClubEngine firstPair, secondPair;
            start(firstPair, effect);
            start(secondPair, effect);
            for (uint32_t time = 0; time < 8000; time += 37) {
                assert(firstPair.render(time, role % 2, pixels));
                assert(secondPair.render(time, role, other));
                assert(memcmp(pixels, other, sizeof(pixels)) == 0);
            }
        }
    }
}
