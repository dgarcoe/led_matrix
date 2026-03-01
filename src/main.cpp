#include <Arduino.h>
#include <FastLED.h>
#include <NimBLEDevice.h>

// --- Hardware config ---
#define LED_PIN     14
#define NUM_LEDS    64
#define BRIGHTNESS  40  // 0-255, keep low for desk use

// --- BLE UUIDs ---
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define STATUS_CHAR_UUID    "12345678-1234-5678-1234-56789abcdef1"

// --- Status values ---
enum Status : uint8_t {
    STATUS_OFF    = 0x00,
    STATUS_GREEN  = 0x01,
    STATUS_YELLOW = 0x02,
    STATUS_RED    = 0x03,
};

CRGB leds[NUM_LEDS];
volatile uint8_t currentStatus = STATUS_OFF;
uint8_t displayedStatus = 0xFF; // force initial update

static CRGB statusToColor(uint8_t status) {
    switch (status) {
        case STATUS_GREEN:  return CRGB(0, 255, 0);
        case STATUS_YELLOW: return CRGB(255, 180, 0);
        case STATUS_RED:    return CRGB(255, 0, 0);
        default:            return CRGB::Black;
    }
}

static const char* statusToName(uint8_t status) {
    switch (status) {
        case STATUS_GREEN:  return "GREEN (Available)";
        case STATUS_YELLOW: return "YELLOW (Busy)";
        case STATUS_RED:    return "RED (Do Not Disturb)";
        default:            return "OFF";
    }
}

// --- BLE Callbacks ---

class StatusCharCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        NimBLEAttValue val = pChar->getValue();
        if (val.size() >= 1 && val[0] <= STATUS_RED) {
            currentStatus = val[0];
            Serial.printf("Status set to: %s\n", statusToName(currentStatus));
        } else {
            Serial.println("Invalid status value received");
        }
    }

    void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
        Serial.println("Status read by client");
    }
};

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        Serial.printf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        Serial.printf("Client disconnected (reason=%d), restarting advertising\n", reason);
        NimBLEDevice::getAdvertising()->start();
    }
};

static StatusCharCallbacks charCallbacks;
static ServerCallbacks serverCallbacks;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Desk Status Display ===");

    // LED setup
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

    // BLE setup
    NimBLEDevice::init("DeskStatus");
    NimBLEDevice::setPower(3); // dBm, enough for desk range

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);

    NimBLEService* pService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic* pStatusChar = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pStatusChar->setCallbacks(&charCallbacks);
    pStatusChar->setValue((uint8_t)currentStatus);
    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("DeskStatus");
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();

    Serial.println("BLE advertising started. Waiting for connections...");
    Serial.println("Write 01=green, 02=yellow, 03=red, 00=off");
}

void loop() {
    uint8_t status = currentStatus;
    if (status != displayedStatus) {
        fill_solid(leds, NUM_LEDS, statusToColor(status));
        FastLED.show();
        displayedStatus = status;
    }
    delay(50);
}
