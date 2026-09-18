#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <device-name> [upload-port]"
  echo "Example: $0 octacore-1 /dev/cu.usbserial-0001"
  exit 1
fi

DEVICE_NAME_VALUE="$1"
UPLOAD_PORT="${2:-}"

if [[ ! "$DEVICE_NAME_VALUE" =~ ^[A-Za-z0-9-]+$ ]]; then
  echo "Device name may only contain letters, numbers, and hyphens."
  exit 1
fi

if [[ ! -f .env ]]; then
  echo ".env not found. Copy .env.sample to .env and configure Wi-Fi first."
  exit 1
fi

export DEVICE_NAME="$DEVICE_NAME_VALUE"

echo "Building and flashing $DEVICE_NAME..."

if [[ -n "$UPLOAD_PORT" ]]; then
  pio run --target upload --upload-port "$UPLOAD_PORT"
else
  pio run --target upload
fi

echo
echo "Flash complete for $DEVICE_NAME."
echo "Open the serial monitor with: pio device monitor --baud 115200"
