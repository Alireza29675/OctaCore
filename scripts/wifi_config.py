"""Generate private credentials without putting values in compiler flags."""
import json
import os
from pathlib import Path

Import("env")

source = Path(env.subst("$PROJECT_DIR")) / "secrets" / "wifi.json"
try:
    config = json.loads(source.read_text(encoding="utf-8"))
except (OSError, ValueError):
    raise SystemExit("Create valid secrets/wifi.json using secrets/wifi.example.json.") from None

if not isinstance(config, dict) or set(config) != {"ssid", "password"}:
    raise SystemExit("Wi-Fi config must contain only ssid and password strings.")
if any(not isinstance(config[key], str) or "\0" in config[key] for key in config):
    raise SystemExit("Wi-Fi config requires strings without null characters.")
try:
    encoded_config = {key: value.encode("utf-8") for key, value in config.items()}
except UnicodeEncodeError:
    raise SystemExit("Wi-Fi config must contain valid UTF-8 strings.") from None
if len(encoded_config["ssid"]) > 32 or len(encoded_config["password"]) > 64:
    raise SystemExit("Wi-Fi SSID or password exceeds the supported byte length.")
if not config["ssid"] and config["password"]:
    raise SystemExit("Wi-Fi password requires a nonempty SSID.")

os.chmod(source, 0o600)
build_directory = Path(env.subst("$BUILD_DIR"))
# Object files and firmware binaries contain the same credentials as the header.
build_directory.mkdir(parents=True, exist_ok=True, mode=0o700)
os.chmod(build_directory, 0o700)
directory = build_directory / "private"
directory.mkdir(parents=True, exist_ok=True, mode=0o700)
os.chmod(directory, 0o700)
header = directory / "WiFiCredentials.h"


def declaration(name, value):
    # Hex escapes preserve UTF-8 bytes without embedding raw credential text.
    encoded = "".join("\\x%02x" % byte for byte in value.encode("utf-8"))
    return 'static const char ' + name + '[] = "' + encoded + '";\n'


content = "#pragma once\n" + declaration("WIFI_SSID", config["ssid"]) + declaration("WIFI_PASSWORD", config["password"])
if not header.exists() or header.read_text() != content:
    descriptor = os.open(header, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(descriptor, "w") as output:
        output.write(content)
env.Append(CPPPATH=[str(directory)])
os.chmod(header, 0o600)
