#include <Arduino.h>

struct Config {
    static constexpr uint8_t BUTTON_PIN = 4;
    static constexpr uint32_t SERIAL_BAUD = 115200;
};

volatile uint32_t isrCounter = 0;

void IRAM_ATTR onButtonFalling() {
    isrCounter++;
}

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(1000); // Пауза, щоб встиг підключитися Serial Monitor (особливо на Native USB)

    pinMode(Config::BUTTON_PIN, INPUT); // Зовнішній pull-up 10 кОм, тому без _PULLUP
    attachInterrupt(digitalPinToInterrupt(Config::BUTTON_PIN), onButtonFalling, FALLING);

    Serial.println("Task 1: Raw FALLING interrupt started. Press the button...");
}

void loop() {
    static uint32_t lastPrintedCount = 0;

    // Зчитуємо volatile-змінну атомарно (32-бітне читання на ESP32-S3 атомарне)
    uint32_t currentCount = isrCounter;

    if (currentCount != lastPrintedCount) {
        Serial.printf("Interrupt count: %lu (+%lu)\n",
                      currentCount,
                      currentCount - lastPrintedCount);
        lastPrintedCount = currentCount;
    }
}

