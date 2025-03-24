#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

#define NUM_LEDS 200           // Número total de LEDs por tira
#define DATA_PIN_1 2           // Pin de datos para el primer par
#define DATA_PIN_2 3           // Pin de datos para el segundo par
#define BRIGHTNESS 200         // Brillo general (0-255)
#define RESET_COMMAND 254       // Comando de reinicio desde Max MSP
const int PACKET_SIZE = 6;

CRGB leds1[NUM_LEDS];         // Array para el primer par de tiras
CRGB leds2[NUM_LEDS];         // Array para el segundo par de tiras
const byte SYNC_MAXMSP  = 255;  // Sync byte para datos enviados desde Max/MSP
byte height;
byte speed;
byte start1;
byte start2;
byte reset;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


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
    //delay(50);
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
        //delay(waveSpeed);
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
    //delay(random(10, 80));
}

bool checkResetSignal() {
    static byte buffer[PACKET_SIZE];
    static byte index = 0;

    // Check if there's enough data in the serial buffer
    while (Serial.available()) {
        byte incomingByte = Serial.read();
        Serial.print("Incoming byte: ");
        Serial.println(incomingByte);
        Serial.println("*******");
        
        // Start reading only if SYNC byte is detected first
        if (index == 0 && incomingByte != SYNC_MAXMSP) {
            continue;  // Ignore invalid data before SYNC
        }

        buffer[index++] = incomingByte;

        // If we've received the full packet, process it
        if (index >= PACKET_SIZE) {
            index = 0; // Reset for the next packet

            // Assign values
            height = buffer[1];  
            speed = buffer[2];  
            start1 = buffer[3];  
            start2 = buffer[4];  
            reset = buffer[5];  

            // Reset check
            if (reset == RESET_COMMAND) {
                startTime = millis();
                phase = 0;

                lcd.clear();
                lcd.setCursor(0, 0);
                return true;  // End the function
            }

            // Print data on LCD
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("H:");
            lcd.print(height);
            lcd.print(" S:");
            lcd.print(speed);
            lcd.setCursor(0, 1);
            lcd.print("S1:");
            lcd.print(start1);
            lcd.print(" S2:");
            lcd.print(start2);
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
    startTime = millis();
    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.print("SimpleSend");
}

void loop() {
    checkResetSignal();
    unsigned long currentTime = millis();
    unsigned long elapsed = (currentTime - startTime) / 1000;
    switch (phase) {
        case 0: phase0.update(elapsed, phase, startTime); break;
        //case 1: phase1.update(elapsed, phase, startTime); break;
        //case 2: phase2.update(elapsed, phase, startTime); break;
        //case 3: phase3.update(elapsed, phase, startTime); break;
    }
}
