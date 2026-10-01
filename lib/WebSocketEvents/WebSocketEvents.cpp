#include "WebSocketEvents.h"
#include "PaletteProtocol.h"
#include "LEDStripController.h"
#include "WebSocketModule.h"
#include "ClubEngine.h"
#include "TowerIdentity.h"

extern LEDStripController ledStrip;
extern WebSocketModule webSocketModule;

static PaletteProtocol protocol;
static ClubEngine club;
static const uint8_t role = OCTACORE_WIRE_ROLE;

static void renderClubNow(uint32_t now) {
    uint8_t pixels[LED_COUNT][3];
    if (!club.render(now, role, pixels)) return;
    ledStrip.setBrightness(club.brightness());
    for (int i = 0; i < LED_COUNT; ++i) {
        ledStrip.setLedColor(i, pixels[i][0], pixels[i][1], pixels[i][2]);
    }
    ledStrip.show();
}

void renderClubFrame() {
    static uint32_t lastFrame = 0;
    const uint32_t now = millis();
    club.tick(now);
    if (static_cast<uint32_t>(now - lastFrame) < 16) return;
    lastFrame = now;
    renderClubNow(now);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type != WStype_BIN || length == 0 || payload == nullptr) return;
    if (payload[0] >= 32) {
        ClubEngine::Reply reply;
        if (!club.handle(payload, length, millis(), role, reply)) return;
        if (payload[0] == 39) ledStrip.clear();
        if (payload[0] == 44) {
            if (payload[5] != 0) ledStrip.clear();
            else renderClubNow(millis());
        }
        if (reply.length != 0) webSocketModule.sendBIN(num, reply.bytes, reply.length);
        return;
    }
    if (!protocol.apply(payload, length)) return;
    if (payload[0] >= 2 && payload[0] <= 5) {
        club.stop();
        ledStrip.setBrightness(protocol.brightness);
    }
    if (payload[0] == 3 || payload[0] == 4) {
        for (int i = 0; i < LED_COUNT; ++i) {
            ledStrip.setLedColor(i, protocol.pixels[i][0], protocol.pixels[i][1], protocol.pixels[i][2]);
        }
        ledStrip.show();
    } else if (payload[0] == 5) {
        ledStrip.setBrightness(protocol.brightness);
        ledStrip.show();
    }
}
