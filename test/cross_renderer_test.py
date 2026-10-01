"""Compare native firmware frames with the actual browser pixelAt function.

Run with DEVELOPER_DIR=/Library/Developer/CommandLineTools on macOS if needed.
An optional argument selects a different lstudio public/app.js checkout.
"""
import json
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
APP = Path(sys.argv[1]) if len(sys.argv) > 1 else (
    ROOT.parent / "lstudio/projects/hightechmess/src/playground/public/app.js"
)
PROBE = r"""
#include <iostream>
#include "ClubEngine.h"
int main() {
    unsigned int byte, role;
    uint64_t elapsed;
    while (std::cin >> elapsed >> role) {
        uint8_t stage[46];
        for (auto& value : stage) {
            if (!(std::cin >> byte)) return 2;
            value = byte;
        }
        ClubEngine engine;
        ClubEngine::Reply reply;
        if (!engine.handle(stage, sizeof(stage), 0, role, reply)) return 3;
        uint8_t commit[9] = {33};
        ClubEngine::write32(commit + 1, ClubEngine::read32(stage + 2));
        if (!engine.handle(commit, sizeof(commit), 0, role, reply)) return 4;
        uint8_t pixels[LED_COUNT][3];
        engine.render(0, role, pixels);
        uint64_t advanced = 0;
        while (elapsed - advanced > UINT32_MAX) {
            advanced += UINT32_MAX;
            engine.tick(static_cast<uint32_t>(advanced));
        }
        engine.render(static_cast<uint32_t>(elapsed), role, pixels);
        for (const auto& pixel : pixels) {
            for (uint8_t channel : pixel) std::cout << unsigned(channel) << ' ';
        }
        std::cout << '\n';
    }
}
"""
JS_PROBE = r"""
const fs = require('fs'), vm = require('vm');
const code = fs.readFileSync(process.argv[1], 'utf8');
const functions = code.slice(code.indexOf('function hash32('), code.indexOf('const canvas'));
if (!functions.includes('function pixelAt(') || !functions.includes('function hash32(')) {
  throw new Error('Renderer function layout changed; update extraction explicitly.');
}
const sandbox = {};
vm.createContext(sandbox);
vm.runInContext(functions, sandbox);
const cases = JSON.parse(fs.readFileSync(0, 'utf8'));
for (const test of cases) {
  const frame = [];
  for (let i = 0; i < 60; i++) {
    const pixel = sandbox.pixelAt(test.program, test.elapsed, test.role * 60 + i);
    for (let channel = 0; channel < 3; channel++) {
      const value = pixel ? parseInt(pixel.color.slice(1 + channel * 2, 3 + channel * 2), 16) : 0;
      frame.push(pixel ? Math.floor(value * pixel.gain) : 0);
    }
  }
  process.stdout.write(frame.join(' ') + '\n');
}
"""


def stage_bytes(program):
    payload = bytearray([32, 1])
    payload.extend((7).to_bytes(4, "little"))
    payload.extend(round(program["bpm"] * 100).to_bytes(2, "little"))
    payload.extend(program[key] for key in ("effect", "brightness", "duty", "division", "dash", "motion"))
    payload.extend(program["seed"].to_bytes(4, "little"))
    for color in program["colors"]:
        payload.extend(bytes.fromhex(color[1:]))
    payload.extend(program["steps"])
    assert len(payload) == 46
    return payload


def main():
    rng = random.Random(42081)
    cases = []
    for effect in range(6):
        for role in range(4):
            for variant in range(12):
                bpm = [40, 127.13, 128, 240][variant % 4]
                division = [1, 2, 4][variant % 3]
                duty = [1, 35, 70, 100][variant % 4]
                program = {
                    "bpm": bpm, "effect": effect, "brightness": 255, "duty": duty,
                    "division": division, "dash": [1, 4, 15][variant % 3],
                    "motion": [1, 4, 16][variant % 3],
                    "seed": [0, 12345, 0xffffffff, rng.getrandbits(32)][variant % 4],
                    "colors": ["#ff285b", "#00e5ff", "#b1ff3b", "#ffffff"],
                    "steps": ([4] * 16 if variant % 2 else [0, 1, 2, 3, 4, 0, 4, 1] * 2),
                }
                step_ms = 60000 / bpm / division
                gate_ms = step_ms * duty / 100
                times = {0, 1, 125, 250, 500, 750, 12345, 3600000,
                         0xfffffff0, 0x100000064, (1 << 40) + 12345}
                for boundary in (gate_ms, step_ms, step_ms * 16):
                    times.update(max(0, int(boundary) + delta) for delta in (-1, 0, 1))
                for elapsed in sorted(times):
                    cases.append({"program": program, "elapsed": elapsed, "role": role})
    native_input = "".join(
        f'{case["elapsed"]} {case["role"]} '
        + " ".join(str(value) for value in stage_bytes(case["program"])) + "\n"
        for case in cases
    )
    with tempfile.TemporaryDirectory(prefix="octacore-renderer-") as temporary:
        source = Path(temporary) / "probe.cpp"
        binary = Path(temporary) / "probe"
        source.write_text(PROBE)
        subprocess.run(
            [os.environ.get("CXX", "c++"), "-std=c++11", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-I" + str(ROOT / "include"), str(source), "-o", str(binary)],
            check=True,
        )
        native = subprocess.check_output([str(binary)], input=native_input, text=True).splitlines()
    browser = subprocess.check_output(
        ["node", "-e", JS_PROBE, str(APP)], input=json.dumps(cases), text=True
    ).splitlines()
    assert len(native) == len(browser) == len(cases)
    differences = 0
    for case, actual_line, expected_line in zip(cases, native, browser):
        actual = [int(value) for value in actual_line.split()]
        expected = [int(value) for value in expected_line.split()]
        assert len(actual) == len(expected) == 180
        tolerance = 1 if case["program"]["effect"] == 5 else 0
        if any(abs(a - b) > tolerance for a, b in zip(actual, expected)):
            raise AssertionError(f'Renderer mismatch: {case}')
        differences += sum(a != b for a, b in zip(actual, expected))
    print(f"PASS: {len(cases)} frames / {len(cases) * 60} pixels; all effects, four towers.")
    print(f"Pulse channels differing by at most one quantization level: {differences}.")


if __name__ == "__main__":
    main()
