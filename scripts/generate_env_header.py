import json
import os
import re
from pathlib import Path

Import("env")

project_dir = Path(env["PROJECT_DIR"])
env_file = project_dir / ".env"
output_file = project_dir / "include" / "EnvGenerated.h"

values = {}

if env_file.exists():
    for raw_line in env_file.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()

        if (
            len(value) >= 2
            and value[0] == value[-1]
            and value[0] in {'"', "'"}
        ):
            value = value[1:-1]

        values[key] = value


def get(name, default=""):
    return os.environ.get(name, values.get(name, default))


device_name = get("DEVICE_NAME", "octacore")
wifi_ssid = get("WIFI_SSID", "")
wifi_password = get("WIFI_PASSWORD", "")
setup_ap_password = get("SETUP_AP_PASSWORD", "octacore-setup")

if not re.fullmatch(r"[A-Za-z0-9-]+", device_name):
    raise ValueError(
        "DEVICE_NAME may only contain letters, numbers, and hyphens."
    )

if setup_ap_password and len(setup_ap_password) < 8:
    raise ValueError("SETUP_AP_PASSWORD must be at least 8 characters.")

try:
    websocket_port = int(get("WEBSOCKET_PORT", "81"))
except ValueError as error:
    raise ValueError("WEBSOCKET_PORT must be an integer.") from error

if not 1 <= websocket_port <= 65535:
    raise ValueError("WEBSOCKET_PORT must be between 1 and 65535.")

header = f"""#pragma once

#include <Arduino.h>

// Generated from .env by scripts/generate_env_header.py.
// Do not edit or commit this file.
namespace OctaEnv {{
inline constexpr const char* DEVICE_NAME = {json.dumps(device_name)};
inline constexpr const char* WIFI_SSID = {json.dumps(wifi_ssid)};
inline constexpr const char* WIFI_PASSWORD = {json.dumps(wifi_password)};
inline constexpr const char* SETUP_AP_PASSWORD = {json.dumps(setup_ap_password)};
inline constexpr uint16_t WEBSOCKET_PORT = {websocket_port};
}}  // namespace OctaEnv
"""

output_file.write_text(header, encoding="utf-8")
print(f"Generated {output_file.relative_to(project_dir)}")
