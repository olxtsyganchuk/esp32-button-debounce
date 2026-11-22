#include <Arduino.h>

struct Config {
    static constexpr uint8_t BUTTON_PIN = 4;
    static constexpr uint32_t SERIAL_BAUD = 115200;
    static constexpr uint32_t POLL_INTERVAL_MS = 5;       // Опитуємо пін кожні 5 мс
    static constexpr uint8_t REQUIRED_STABLE_SAMPLES = 6; // 6 * 5 мс = 30 мс стабільного стану
};

enum class ButtonState {
    IDLE,           // Кнопка відпущена (очікуємо натискання)
    DEBOUNCE_PRESS, // Зафіксовано LOW, перевіряємо стабільність
    PRESSED,        // Натискання підтверджено, кнопка утримується
    DEBOUNCE_RELEASE // Зафіксовано HIGH, перевіряємо стабільність відпускання
};

void setup() {
    Serial.begin(Config::SERIAL_BAUD);
    delay(1000);

    pinMode(Config::BUTTON_PIN, INPUT); // Ніяких attachInterrupt!

    Serial.println("Task 4: Polling (5 ms) + FSM debounce started...");
}

void loop() {
    static uint32_t lastPollTime = 0;
    static ButtonState state = ButtonState::IDLE;
    static uint8_t stableCounter = 0;
    static uint32_t validPressCount = 0;

    uint32_t now = millis();

    // Виконуємо крок опитування строго кожні 5 мс
    if (now - lastPollTime < Config::POLL_INTERVAL_MS) {
        return;
    }
    lastPollTime = now;

    bool isPinLow = (digitalRead(Config::BUTTON_PIN) == LOW);

    switch (state) {
        case ButtonState::IDLE:
            if (isPinLow) {
                stableCounter = 1;
                state = ButtonState::DEBOUNCE_PRESS;
            }
            break;

        case ButtonState::DEBOUNCE_PRESS:
            if (isPinLow) {
                stableCounter++;
                if (stableCounter >= Config::REQUIRED_STABLE_SAMPLES) {
                    state = ButtonState::PRESSED;
                    validPressCount++;
                    Serial.printf("Valid press: %lu (FSM confirmed)\n", validPressCount);
                }
            } else {
                // Випадковий сплеск або брязкіт -> повертаємось у спокій
                stableCounter = 0;
                state = ButtonState::IDLE;
            }
            break;

        case ButtonState::PRESSED:
            if (!isPinLow) {
                stableCounter = 1;
                state = ButtonState::DEBOUNCE_RELEASE;
            }
            break;

        case ButtonState::DEBOUNCE_RELEASE:
            if (!isPinLow) {
                stableCounter++;
                if (stableCounter >= Config::REQUIRED_STABLE_SAMPLES) {
                    stableCounter = 0;
                    state = ButtonState::IDLE;
                }
            } else {
                // Під час відпускання контакт ще брязнув у LOW -> повертаємось у PRESSED
                stableCounter = 0;
                state = ButtonState::PRESSED;
            }
            break;
    }
}