#include <FastLED.h>
#include <Arduino.h>

#define NUM_LEDS 200           // Número total de LEDs por tira
#define DATA_PIN_1 3           // Pin de datos para el primer par
#define DATA_PIN_2 6           // Pin de datos para el segundo par
#define BRIGHTNESS 200         // Brillo general (0-255)
#define RESET_COMMAND 255       // Comando de reinicio desde Max MSP
#define LED_BUILTIN 13

CRGB leds1[NUM_LEDS];         // Array para el primer par de tiras
CRGB leds2[NUM_LEDS];         // Array para el segundo par de tiras


// Variables globales
int phase = 0;
unsigned long startTime;
int tempBrightness = 0;

class PhaseHandler {
public:
    using EffectFunction = void(*)();

    PhaseHandler(EffectFunction effect, unsigned long duration, int nextPhase)
        : effect(effect), duration(duration), nextPhase(nextPhase) {}

    bool update(unsigned long elapsed, int &phase, unsigned long &startTime) {
        if (elapsed < duration) {
            effect();
            return false;
        }
        phase = nextPhase;
        startTime = millis();
        return true;
    }

private:
    EffectFunction effect;
    unsigned long duration;
    int nextPhase;
};

void offAndWait() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(50);
}

void showContemplativeEffect() {
    const int waveSize = 20;
    const int maxBrightness = 255;
    const int minBrightness = tempBrightness;
    const int waveSpeed = 50;
    bool stopLoop = false;

    fill_solid(leds1, NUM_LEDS, CRGB(0, minBrightness, 0));
    fill_solid(leds2, NUM_LEDS, CRGB(0, minBrightness, 0));
    for (int i = 0; i < NUM_LEDS; i++) {
        for (int j = 0; j < waveSize; j++) {
            int ledIndex = (i - j + NUM_LEDS) % NUM_LEDS;
            float positionFactor = abs((waveSize / 2.0) - j) / (waveSize / 2.0);
            int brightness = minBrightness + (maxBrightness - minBrightness) * (1.0 - positionFactor);
            leds1[ledIndex] = CRGB(0, brightness, 0);
            leds2[ledIndex] = CRGB(0, brightness, 0);
            if (checkResetSignal()) {
                stopLoop = true;
                break;
            }
        }
        if (stopLoop) break;
        FastLED.show();
        delay(waveSpeed);
    }
    FastLED.show();
}

void showChaosEffect() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    int maxWidthFlash = 5;
    int numSegments1 = random(2, maxWidthFlash);
    int numSegments2 = random(2, maxWidthFlash);

    for (int j = 0; j < numSegments1; j++) {
        int startIndex = random(NUM_LEDS - maxWidthFlash);
        for (int k = 0; k < maxWidthFlash; k++) {
            leds1[startIndex + k] = CRGB::White;
        }
    }
    for (int j = 0; j < numSegments2; j++) {
        int startIndex = random(NUM_LEDS - maxWidthFlash);
        for (int k = 0; k < maxWidthFlash; k++) {
            leds2[startIndex + k] = CRGB::White;
        }
    }
    FastLED.show();
    delay(random(10, 80));
}

bool checkResetSignal() {
    if (Serial.available() > 0) {
        int command = Serial.read();
        tempBrightness = command;
        Serial.print(command);
        Serial.print('\n');
        if (command == RESET_COMMAND) {
            startTime = millis();
            Serial.println(command);
            phase = 0;
            digitalWrite(LED_BUILTIN, HIGH);
            digitalWrite(LED_BUILTIN, LOW);
            Serial.println("Reset signal received");
            return true;
        }
    }
    return false;
}

PhaseHandler phase0(showContemplativeEffect, 10, 1);
PhaseHandler phase1(offAndWait, 3, 2);
PhaseHandler phase2(showChaosEffect, 5, 3);
PhaseHandler phase3(offAndWait, 3, 0);

void setup() {
    FastLED.addLeds<WS2811, DATA_PIN_1, GRB>(leds1, NUM_LEDS);
    FastLED.addLeds<WS2811, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(14400);
    startTime = millis();
}

void loop() {
    checkResetSignal();
    unsigned long currentTime = millis();
    unsigned long elapsed = (currentTime - startTime) / 1000;
    switch (phase) {
        case 0: phase0.update(elapsed, phase, startTime); break;
        case 1: phase1.update(elapsed, phase, startTime); break;
        case 2: phase2.update(elapsed, phase, startTime); break;
        case 3: phase3.update(elapsed, phase, startTime); break;
    }
}
