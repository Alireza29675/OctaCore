# OctaCore

Firmware for one OctaCore ESP32 lighting module.

Each module controls:

- one 60-pixel WS2812 strip on GPIO 5
- one servo on GPIO 15
- a WebSocket server (port 81 by default)
- Wi-Fi + mDNS

The matching controller is the `projects/hightechmess` project in
[Alireza29675/lstudio](https://github.com/Alireza29675/lstudio).

## Configuration

Copy the sample environment file:

```bash
cp .env.sample .env
```

Edit `.env`:

```dotenv
DEVICE_NAME=octacore-1
WIFI_SSID=your-wifi
WIFI_PASSWORD=your-password
SETUP_AP_PASSWORD=a-password-of-at-least-8-characters
WEBSOCKET_PORT=81
```

Use a different `DEVICE_NAME` for each physical controller, for example
`octacore-1` through `octacore-4`.

The `.env` file is never committed. PlatformIO generates
`include/EnvGenerated.h` at build time.

If `WIFI_SSID` is empty, the ESP32 first tries credentials already stored on
the device. If it cannot connect, it opens a WiFiManager setup access point
named after `DEVICE_NAME`.

## Build

Install PlatformIO Core, then:

```bash
pio run
```

## Flash

Connect one ESP32 over USB:

```bash
pio device list
pio run --target upload
pio device monitor
```

The serial monitor runs at 115200 baud and prints the assigned IP address and
mDNS endpoint.

After flashing a device named `octacore-1`, it should normally be reachable
at:

```text
ws://octacore-1.local:81
```

## Protocol

Binary WebSocket messages use the first byte as the command ID:

| ID | Command | Payload |
| --- | --- | --- |
| 1 | rotate servo | 1 byte angle (0–180) |
| 2 | set palette | up to 16 RGB triples |
| 3 | set LED colors | 30 bytes, two 4-bit palette indices per byte |
| 4 | fill strip | 1 palette index |
| 5 | brightness | 1 byte (0–255) |
| 15 | reset Wi-Fi | no payload |

For command 3, the even LED is stored in the low nibble and the odd LED in the
high nibble. This matches `lstudio`.

## Legacy folder

The `prototype/` directory contains the old Arduino-IDE implementation kept
for historical reference. The supported firmware is now the PlatformIO code in
`src/`, `include/`, and `lib/`.
