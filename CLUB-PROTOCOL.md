# Buffered club protocol v2

One binary WebSocket message per command on port81. All multibyte integers
are little endian. Programs remain in RAM and loop locally through Wi-Fi
loss; reboot starts black. Legacy commands1–5 remain available.

| Opcode | Total bytes | Fields following opcode |
| --- | --- | --- |
|32 STAGE|46|version1:u8, revision:u32, bpm×100:u16, effect:u8, brightness:u8, duty:u8, division:u8, dash:u8, motion:u8, seed:u32, colors:12 bytes, steps:16 bytes|
|33 COMMIT|9|revision:u32, startAtDeviceMillis:u32|
|34 CLOCK|5|token:u32|
|35 CLOCK reply|9|token:u32, deviceMillis:u32|
|36 HELLO|1|none|
|37 HELLO reply|7|version2:u8, role:u8 (tower number minus 1: 0–3), activeRevision:u32 (0when stopped)|
|38 ACK|6|revision:u32, status:u8 (0staged/1committed)|
|39 STOP|1|none|
|40 TEMPO|15|tempoRevision:u32, bpm×100:u16, anchorDeviceMillis:u32, beatNumber:u32|
|41 TEMPO ACK|5|tempoRevision:u32|
|44 HOLD BLACKOUT|6|revision:u32, held:u8 (0/1)|
|45 HOLD ACK|6|revision:u32, held:u8 (0/1)|

HOLD BLACKOUT is momentary: held1 immediately blacks out while the active
program, pending activation, and independent clock continue. Held0 renders
the current phase immediately without restarting or staging anything.
ACK45 echoes the revision and held value. Malformed frames are ignored
atomically. STOP remains permanent, clears the held flag and all programs,
and a subsequent release cannot revive a stopped program. The host sends
release without waiting for the press ACK and correlates status by revision.

HELLO advertises version2; STAGE retains its version1 program schema.
Tower 1 has wire role0 (legacy left), tower 2 role1 (legacy right), tower 3
role2, and tower 4 role3. These identities remain distinct even when their
rendered patterns match. Packet sizes and clock behavior are unchanged.
TEMPO anchors an independent beat clock, without staging, activating, or
changing an effect. Tempo must be40–240BPM and the anchor within±10000ms of
receipt, using signed uint32 differences. Malformed packets are rejected
atomically. ACK41 echoes the accepted tempo revision. A late valid packet
uses its original anchor rather than receipt time. The host serializes tempo
updates and matches ACK revisions; firmware accepts valid revisions without
assuming an ordering across host restarts.

STOP and legacy overrides retain the clock. Effect activation retains its
tempo and absolute beat count. When no clock has ever been initialized, the
first program activation initializes one from its scheduled start and BPM.
Subsequent programs cannot reset it. A reboot clears both programs and clock.
The host normalizes older anchors to a recent whole beat and carries that
beatNumber, preserving deterministic random steps across effect changes.

Validate complete packets before state changes. Tempo40–240BPM, effect0–5,
brightness0–255, duty1–100%, division1/2/4, dash1–15, motion1–16, steps0–4.
Stage leaves the active program running. Commit copies the matching staged
program to a pending slot; later staging cannot alter that pending program.
Start must be within now through now+10000ms, using signed uint32 differences
for wrap safety. ACK1 confirms scheduling, not that playback has started.
STOP clears active/staged/pending state and blacks out immediately. Disconnect
does not restart or stop a program. Valid legacy commands2–5 cancel local
playback; ignored servo command1 does not. Invalid commands do not cancel it.

The host estimates each board's clock offset from the midpoint of three
clock round trips, selects the best RTT, then schedules both at one host
epoch at least750ms ahead (preferably the next playing beat). Stage and commit
ACKs are mandatory. HELLO gates compatibility; never silently substitute
network frame streaming when club firmware is unavailable.

## Renderer agreement

Render60 pixels per role. `g=(role%2)*60+i` places each tower within the
established 120-pixel two-tower pattern. Towers 3/4 repeat towers 1/2 for every
effect, including deterministic sparks. This intentionally preserves the exact
two-tower behavior and keeps Bounce visible on all four towers. There is no
active-tower-count or 240-pixel layout field in this protocol.
Calculate from signed extended milliseconds since the independent tempo anchor:

```text
beat = beatNumber + elapsedMs * bpm / 60000
position = beat * division
absoluteStep = floor(position)
fraction = position - floor(position)
cell = steps[absoluteStep % 16]
dutyFraction = duty / 100
```

Black before beat zero, if cell0, or fraction>=dutyFraction. Cells1/2/3 choose colors0/1/2;
cell4 chooses `hash32(seed ^ uint32(absoluteStep)) % 4`.
Hash uint32 arithmetic (JS uses Math.imul and unsigned shifts):

```text
x ^= x >>> 16
x = imul(x, 0x7feb352d)
x ^= x >>> 15
x = imul(x, 0x846ca68b)
x ^= x >>> 16
return uint32(x)
```

Within the visible portion of a step:

|Effect|Pixels lit|
|---|---|
|0 Strobe|all|
|1 Dash chase|`(g + floor(beat*motion*4)) % (dash*2) < dash`|
|2 Bounce|`abs(g - head) < dash`, where phase=(beat*motion/4)%2 and head=(phase<=1?phase:2-phase)*119|
|3 Orbit|`(g + floor(beat*motion*8)) % 120 < dash*3`|
|4 Sparks|`hash32(seed ^ uint32(absoluteStep) ^ imul(g+1,0x9e3779b9)) % 16 < dash`|
|5 Pulse|all; gain=`(1-fraction/dutyFraction)^2`|

RGB channels are `floor(colorChannel*gain)`; gain1 for effects0–4.
Program brightness is applied separately through FastLED's existing global
brightness control. Preview should multiply by brightness/255 for display;
FastLED's byte scaling and physical LED response may differ slightly.
No extra fade tails. Frame cadence is approximately60fps and never advances
the step index itself. Extended integer elapsed time survives millis rollover.
