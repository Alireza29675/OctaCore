# OctaCore local recovery

Last verified: 2026-09-18. Run commands from this repository.

## Private Wi-Fi configuration

Edit `secrets/wifi.json` locally in an editor:

The historical `.env.sample` is unused; only this JSON config is read.

```json
{
  "ssid": "",
  "password": ""
}
```

If missing, copy `secrets/wifi.example.json` to `secrets/wifi.json` and run
`chmod 600 secrets/wifi.json`. Do not paste credentials into chat, shell
arguments, source, or environment flags. JSON requires quotes and backslashes
inside values to be escaped. Use a 2.4 GHz network supported by the ESP32.

The entire secrets directory is ignored except its empty sample. The build
reads the private JSON and writes a 0600 header inside the ignored
`.pio/build/<role>/private/` directory. Diagnostics omit credential values.
Generated firmware binaries and object files contain the credentials. Each
role's build directory is restricted to 0700 as well; never publish build artifacts, and delete local artifacts
when they are no longer needed. Editing credentials requires rebuilding and
uploading both boards. Blank SSID/password compiles successfully and stays
offline. Invalid or missing JSON fails the build without printing its content.

## Build and test without touching hardware

```sh
uv tool run --from platformio pio run -e left -e right
uv tool run --from platformio pio run -e tower1 -e tower2 -e tower3 -e tower4
python3 -m unittest discover -s test -p 'test_wifi_config.py' -v
c++ -std=c++11 -fsanitize=address,undefined -Iinclude test/protocol_test.cpp -o /tmp/octacore-protocol-test
/tmp/octacore-protocol-test
c++ -std=c++11 -fsanitize=address,undefined -Iinclude test/club_engine_test.cpp -o /tmp/octacore-club-test
/tmp/octacore-club-test
python3 test/cross_renderer_test.py
```

On a Mac where Xcode's license prompt blocks the compiler or Git, prefix
build/test commands with `DEVELOPER_DIR=/Library/Developer/CommandLineTools`
when those command line tools are installed.

## Upload only after hardware verification and authorization

Keep the target tower stopped during recovery (entering its USB bootloader stops its program). The optional-tower playground may remain open while a separately identified USB board is backed up/flashed; other towers may continue their existing playback. Do not press Send during the upload. A newly discovered or rebooted tower stays stopped until the next Send. Stop any legacy frame-streaming controller completely. Verify the board, LED pin/count/order,
external LED power supply, and common ground. Disconnect servo power. Identify
each physical board and its serial port, and make a private backup of its
existing flash before replacing it. Flash backup/restore commands depend on
the detected board and flash size; determine those before proceeding.

For an identified original ESP32 with 4 MB flash, the verified backup sequence is:

```sh
uv tool run --from esptool==4.5.1 esptool.py --port /dev/cu.YOUR_PORT --after no_reset flash_id
uv tool run --from esptool==4.5.1 esptool.py --chip esp32 --port /dev/cu.YOUR_PORT --baud 460800 --after no_reset read_flash 0 0x400000 /PRIVATE_BACKUP_DIRECTORY/original-flash.bin
uv tool run --from esptool==4.5.1 esptool.py --chip esp32 --port /dev/cu.YOUR_PORT --baud 460800 --after no_reset verify_flash 0 /PRIVATE_BACKUP_DIRECTORY/original-flash.bin
```

Create the backup directory outside Git with mode0700 and restrict the backup
file to0600. It may contain old network credentials; do not inspect or publish
its contents. Verify the file is exactly4194304 bytes and keep a SHA-256 digest.
`--after no_reset` leaves the board in the bootloader between steps, avoiding
an intervening launch of the old application. Before each write, recheck the
port and physical role; this workflow must never guess between two boards.
Rollback, if explicitly needed, writes this verified full backup at offset0
using `write_flash 0 /PRIVATE_BACKUP_DIRECTORY/original-flash.bin`, with the
same chip/port selection; it restores the old software and its old behavior.

Connect one board at a time and explicitly choose its role and port:

```sh
uv tool run --from platformio pio run -e left -t upload --upload-port /dev/cu.YOUR_LEFT_PORT
uv tool run --from platformio pio run -e right -t upload --upload-port /dev/cu.YOUR_RIGHT_PORT
```

Uploading resets the ESP32. Do not select an upload target just to build.
Only one board per role may be present on the LAN. Label the hardware.
After upload, expect black LEDs until a controller sends frames; no servo
initialization or pulse output is included. Check serial/network status
before starting lstudio. Opening a serial monitor can itself reset the board.
If wiring, role, or network identity is uncertain, stop the controller and
disconnect power; use the verified private backup for rollback.

## Discover the two boards

The names are `octacore-left.local` and `octacore-right.local`.
WebSocket endpoints are `ws://octacore-left.local:81` and
`ws://octacore-right.local:81`. The firmware advertises `_octacore._tcp`
port 81 with TXT `role=left|right`, `tower=1|2`, and `protocol=palette-v1`.
On macOS:

```sh
dns-sd -B _octacore._tcp local.
dns-sd -G v4 octacore-left.local
dns-sd -G v4 octacore-right.local
```

These commands remain running until Ctrl-C. Wi-Fi and mDNS retry after a
connection loss. Both boards and the Mac must share a LAN that permits
multicast; guest isolation, VLAN boundaries, VPNs or multicast filters can
prevent discovery. macOS has native Bonjour. If discovery fails, obtain the
DHCP address from the router or serial log and use lstudio's endpoint override.
Do not expose port 81 to the Internet: the local control protocol has no
authentication. Network announcement and physical behavior require a separate
post-upload check; compilation does not verify either.

## Optional numbered towers and future USB enrollment

The default build still selects `left` and `right`. Existing flashed boards
and their hostnames need no migration to keep the two-tower setup working.
Numbered targets provide canonical names for future enrollment:

| Build target | Tower label | HELLO wire role | Hostname |
| --- | --- | --- | --- |
| left | 1 | 0 | octacore-left.local |
| right | 2 | 1 | octacore-right.local |
| tower1 | 1 | 0 | octacore-1.local |
| tower2 | 2 | 1 | octacore-2.local |
| tower3 | 3 | 2 | octacore-3.local |
| tower4 | 4 | 3 | octacore-4.local |

`left` and `tower1` are alternative builds for the same slot; likewise
`right` and `tower2`. Never enroll both alternatives as separate towers.
The controller can resolve canonical names with legacy left/right fallback
for slots 1/2. Each board advertises one hostname; firmware does not create
an additional mDNS alias. TXT `tower=1..4` identifies the numbered slot,
and the ready serial line includes the tower number. TXT `role` remains
the build's hostname suffix (`left`, `right`, or `1..4`).

For a future tower 3 or 4: identify the connected USB board and its physical
label, verify wiring/power, privately back up and verify its flash using the
procedure above, then explicitly upload its matching target. For example:

```sh
uv tool run --from platformio pio run -e tower3 -t upload --upload-port /dev/cu.YOUR_TOWER3_PORT
uv tool run --from platformio pio run -e tower4 -t upload --upload-port /dev/cu.YOUR_TOWER4_PORT
```

These upload commands are for an authorized future enrollment, not build
verification. Build output for every target remains inside the private ignored
`.pio/build/<target>/` directory and contains Wi-Fi credentials. After upload,
verify the matching numbered hostname, HELLO version2 and wire role before
enabling that tower in the controller. Missing optional towers should stay
disabled. All six effects repeat the existing pair: tower 3 matches 1 and
tower 4 matches 2, with independent identities and the same shared beat clock.

## Protocol compatibility

Each WebSocket binary message is one complete command. Fragmented WebSocket
messages, text, unknown commands, and invalid lengths are ignored.

| Command | Payload after command byte |
| --- | --- |
| 1 | One legacy servo byte, ignored |
| 2 | 48 RGB bytes for 16 palette entries |
| 3 | 30 packed palette-index bytes: even LED low nibble, odd LED high nibble |
| 4 | One palette index, 0–15, fills all LEDs |
| 5 | One brightness byte, 0–255 |

Malformed commands leave all state unchanged. Palette updates take effect on
the next pixel/fill command; brightness changes display immediately.
There is no remote command to erase Wi-Fi credentials or reset the board.
On Wi-Fi loss the last LED output is retained; restarting boots black.

## Buffered BPM playground

Use lstudio's `playground` command for the new browser editor. Stop the legacy
controller first so its frame commands cannot cancel the buffered program.
Effect editing remains local; tempo/alignment updates send automatically. Send & play explicitly stages and schedules
the selected compatible towers. The browser reports a firmware upgrade requirement
when HELLO support is absent. See [CLUB-PROTOCOL.md](CLUB-PROTOCOL.md).

Firmware renders buffered programs locally at approximately60fps, starting
from the committed timestamp. Wi-Fi sleep is disabled to reduce control
latency. Wi-Fi loss leaves the local program playing at its own clock rate;
clock drift during long disconnected sessions is possible. Reconnection alone
never starts a new program. Momentary blackout (opcode44) preserves playback and its clock, and release renders
the current phase immediately. Stop (opcode39) clears pending/active programs and blacks
out reachable boards while keeping the clock; if a board is offline, disconnect its power to stop it.
Programs are not persisted across power cycles.

Before a future upload, identify each selected board role and follow the backup and
upload procedure above. After upload, verify HELLO version2 and correct role,
then stage a low-brightness slow program and verify synchronized starts,
Stop, and behavior through a network interruption. Build/native tests do not
establish physical timing, Wi-Fi behavior, or optical brightness.

The cross-renderer test requires Node and the sibling lstudio checkout; an
optional argument selects its `public/app.js`. It compares the actual browser
renderer with native firmware at beat/duty boundaries and extended uptimes.
This validates the mathematical rendering, not oscillator accuracy. There is
no continuous resynchronization: relative clock error of `e` parts per million
accumulates `3.6 * e` milliseconds per hour (for example100ppm yields360ms/hour).
The actual boards' error has not been measured. Explicit tempo updates establish
a shared beat clock; sending effects preserves that clock; initial alignment also depends on network clock-sampling
error and the approximately16ms render interval.
