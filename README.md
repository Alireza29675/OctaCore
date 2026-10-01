# OctaCore

ESP32 firmware for up to four LED towers controlled by lstudio.
Towers 1/2 retain the existing left/right identities; towers 3/4 are optional.
Each board drives 60 WS2812 LEDs on GPIO 5 in RGB order. Servo support is
disabled in the active build, including its dependency and initialization.
The historical servo and prototype sources are retained for reference.

The LEDs start black. Wi-Fi uses station mode without a provisioning access
point. Blank configuration leaves the board offline and black. Existing
hardware wiring and power must be verified before uploading this firmware.

See [the runbook](RUNBOOK.md) for private Wi-Fi configuration, builds,
role-specific upload instructions and local network discovery.

The lstudio club playground can stage a 16-step BPM program and schedule the
selected towers to start together. Patterns render locally through Wi-Fi interruptions.
The established 120-pixel choreography repeats in pairs: tower 3 matches tower 1,
and tower 4 matches tower 2. Adding optional towers does not stretch or change
the existing two-tower effects.
See [the buffered protocol](CLUB-PROTOCOL.md) for timing and renderer formulas.
