#include <Arduino.h>

struct Config {
    static constexpr uint8_t BUTTON_PIN = 4;
    static constexpr uint32_t SERIAL_BAUD = 115200;
    static constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
};

volatile bool isrTriggered = false;
volatile uint32_t rawIsrCount = 0;

void IRAM_ATTR onButtonFalling() {
    isrTriggered = true;
    rawIsrCount++;
}

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(1000);

    pinMode(Config::BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(Config::BUTTON_PIN), onButtonFalling, FALLING);

    Serial.println("Task 2: Time-based debounce (50 ms outside ISR) started...");
}

void loop() {
    static uint32_t validPressCount = 0;
    static uint32_t lastValidTime = 0;

    if (isrTriggered) {
        isrTriggered = false;
        uint32_t now = millis();

        if (now - lastValidTime >= Config::DEBOUNCE_DELAY_MS) {
            validPressCount++;
            lastValidTime = now;

            Serial.printf("Valid press: %lu | Raw interrupts: %lu\n",
                          validPressCount,
                          rawIsrCount);
        }
    }
}