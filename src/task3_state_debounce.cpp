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

    Serial.println("Task 3: State-based debounce started...");
}

void loop() {
    static uint32_t validPressCount = 0;
    static bool waitingForDebounce = false;
    static uint32_t eventStartTime = 0;
    static bool isButtonPressed = false;

    // 1. Отримали сигнал із переривання -> запускаємо таймер стабілізації
    if (isrTriggered) {
        isrTriggered = false;
        if (!waitingForDebounce) {
            waitingForDebounce = true;
            eventStartTime = millis();
        }
    }

    // 2. Через 50 мс перевіряємо реальний фізичний стан кнопки
    if (waitingForDebounce && (millis() - eventStartTime >= Config::DEBOUNCE_DELAY_MS)) {
        waitingForDebounce = false;

        if (digitalRead(Config::BUTTON_PIN) == LOW) {
            // Кнопка досі фізично натиснута -> зараховуємо, якщо ще не зарахували це натискання
            if (!isButtonPressed) {
                isButtonPressed = true;
                validPressCount++;
                Serial.printf("Valid press: %lu | Raw interrupts: %lu\n",
                              validPressCount,
                              rawIsrCount);
            }
        } else {
            // Рівень HIGH -> кнопку відпустили (або це був брязкіт при відпусканні)
            isButtonPressed = false;
        }
    }

    // 3. Скидаємо стан утримання, коли кнопку відпустили без брязкоту на спад
    if (isButtonPressed && !waitingForDebounce && (digitalRead(Config::BUTTON_PIN) == HIGH)) {
        isButtonPressed = false;
    }
}