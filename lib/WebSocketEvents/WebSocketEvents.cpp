#include "WebSocketEvents.h"

#include "Config.h"
#include "Globals.h"
#include "Protocol.h"

namespace {

void logInvalidLength(
    OctaCoreCommand command,
    size_t actual,
    size_t expected) {
  Serial.print("Invalid payload length for command ");
  Serial.print(static_cast<uint8_t>(command));
  Serial.print(": got ");
  Serial.print(actual);
  Serial.print(", expected ");
  Serial.println(expected);
}

void handleBinaryPayload(uint8_t* payload, size_t length) {
  if (length == 0) {
    return;
  }

  const auto command = static_cast<OctaCoreCommand>(payload[0]);

  switch (command) {
    case OctaCoreCommand::RotateServo: {
      constexpr size_t expectedLength = 2;
      if (length != expectedLength) {
        logInvalidLength(command, length, expectedLength);
        return;
      }

      servoMotor.setAngle(payload[1]);
      break;
    }

    case OctaCoreCommand::SetColorPalette: {
      if ((length - 1) % 3 != 0) {
        Serial.println("Invalid color palette payload.");
        return;
      }

      const size_t colorCount = (length - 1) / 3;
      if (colorCount > COLOR_PALETTE_SIZE) {
        Serial.println("Color palette is too large.");
        return;
      }

      uint32_t colors[COLOR_PALETTE_SIZE] = {};
      for (size_t i = 0; i < colorCount; ++i) {
        const size_t offset = 1 + (i * 3);
        colors[i] =
            (static_cast<uint32_t>(payload[offset]) << 16) |
            (static_cast<uint32_t>(payload[offset + 1]) << 8) |
            static_cast<uint32_t>(payload[offset + 2]);
      }

      ledStrip.setColorPalette(colors);
      break;
    }

    case OctaCoreCommand::SetLedColors: {
      constexpr size_t packedLedBytes = (LED_COUNT + 1) / 2;
      constexpr size_t expectedLength = 1 + packedLedBytes;

      if (length != expectedLength) {
        logInvalidLength(command, length, expectedLength);
        return;
      }

      for (uint16_t ledIndex = 0; ledIndex < LED_COUNT; ++ledIndex) {
        const uint8_t packed = payload[1 + (ledIndex / 2)];

        // lstudio packs even LEDs in the low nibble and odd LEDs in
        // the high nibble.
        const uint8_t paletteIndex =
            ledIndex % 2 == 0
                ? packed & 0x0F
                : (packed >> 4) & 0x0F;

        ledStrip.setLedColor(ledIndex, paletteIndex);
      }

      ledStrip.show();
      break;
    }

    case OctaCoreCommand::FillLeds: {
      constexpr size_t expectedLength = 2;
      if (length != expectedLength) {
        logInvalidLength(command, length, expectedLength);
        return;
      }

      ledStrip.fill(payload[1]);
      ledStrip.show();
      break;
    }

    case OctaCoreCommand::SetLedBrightness: {
      constexpr size_t expectedLength = 2;
      if (length != expectedLength) {
        logInvalidLength(command, length, expectedLength);
        return;
      }

      ledStrip.setBrightness(payload[1]);
      ledStrip.show();
      break;
    }

    case OctaCoreCommand::ResetWifiSettings: {
      Serial.println("Received Wi-Fi reset command.");
      wifiModule.resetSettings();
      break;
    }

    default:
      Serial.print("Unknown command: ");
      Serial.println(payload[0]);
      break;
  }
}

}  // namespace

void onWebSocketEvent(
    uint8_t clientNumber,
    WStype_t type,
    uint8_t* payload,
    size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.print("WebSocket client connected: ");
      Serial.println(clientNumber);
      break;

    case WStype_DISCONNECTED:
      Serial.print("WebSocket client disconnected: ");
      Serial.println(clientNumber);
      break;

    case WStype_BIN:
      handleBinaryPayload(payload, length);
      break;

    default:
      break;
  }
}
