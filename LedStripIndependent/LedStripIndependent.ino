#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

#define NUM_LEDS 150           // Número total de LEDs por tira
#define DATA_PIN_1 2           // Pin de datos para el primer par
#define DATA_PIN_2 3           // Pin de datos para el segundo par
#define BRIGHTNESS 200         // Brillo general (0-255)
const int PACKET_SIZE = 6;
uint8_t buffer[PACKET_SIZE]; // Incoming data buffer
uint8_t lastPackage[PACKET_SIZE] = {0}; // Store the last received packet

CRGB leds1[NUM_LEDS];         // Array para el primer par de tiras
CRGB leds2[NUM_LEDS];         // Array para el segundo par de tiras
const byte SYNC_MAXMSP  = 255;  // Sync byte para datos enviados desde Max/MSP
const byte RESET_MAXMSP = 254;  // Byte de reinicio para datos enviados desde Max/MSP
byte height;
byte speed;
byte start1;
byte start2;
byte reset;
byte reset_flag = 0;

 // Timing
unsigned long lastFrameTime = 0;
const int frameDelay = 100;

bool packetReady = false;
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
}

void showContemplativeEffect() {
    static int i = 0;  // Índice del LED actual en la animación
    static bool stopLoop = false;

    if (stopLoop) {
        i = 0;  // Reiniciar animación para la próxima ejecución
        stopLoop = false;
        return;
    }

    const int waveSize = 20;
    const int maxBrightness = 255;
    const int minBrightness = tempBrightness;

    for (int j = 0; j < waveSize; j++) {
        int ledIndex = (i - j + NUM_LEDS) % NUM_LEDS;
        float positionFactor = abs((waveSize / 2.0) - j) / (waveSize / 2.0);
        int brightness = minBrightness + (maxBrightness - minBrightness) * (1.0 - positionFactor);
        leds1[ledIndex] = CRGB(0, brightness, 0);
        leds2[ledIndex] = CRGB(0, brightness, 0);
    }

    FastLED.show();

    if (checkResetSignal()) {
        stopLoop = true;
        return;
    }

    i++;  // Avanzar al siguiente LED en la próxima iteración

    if (i >= NUM_LEDS) {
        i = 0;  // Reiniciar animación cuando llegue al final
        stopLoop = true;
    }
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
}

bool checkResetSignal() {
    if (reset_flag == RESET_MAXMSP) {  // Check reset byte
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reset");
        delay(1000);
        fill_solid(leds1, NUM_LEDS, CRGB::Black);
        fill_solid(leds2, NUM_LEDS, CRGB::Black);
        FastLED.show();
        startTime = millis();
        phase = 0; // Reset phase
        reset_flag = 0;
        return true;
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
    Serial.begin(460800);
    lcd.begin(16, 2);
    lcd.print("SimpleSend");
}

void loop() {
    unsigned long currentTime = millis();
    unsigned long elapsed = (currentTime - startTime) / 1000;
    lcd.setCursor(0, 1);
    lcd.print(currentTime);
    if (currentTime - lastFrameTime >= frameDelay) {
        lastFrameTime = currentTime;
        switch (phase) {
            case 0: phase0.update(elapsed, phase, startTime); break;
            case 1: phase1.update(elapsed, phase, startTime); break;
            case 2: phase2.update(elapsed, phase, startTime); break;
            case 3: phase3.update(elapsed, phase, startTime); break;
        }
    }
}

// This function is triggered when serial data arrives
void serialEvent() {
    if (Serial.available() >= PACKET_SIZE) {  // Ensure we have a full packet
        Serial.readBytes(buffer, PACKET_SIZE); // Read full packet
        if(buffer[0] == SYNC_MAXMSP) {  // Ensure first byte is 255
            if (memcmp(lastPackage, buffer, PACKET_SIZE) == 0) {
                return;  // Ignore repeated packets
            }
            if((buffer[3] != 0 && buffer[3] != 1) || (buffer[4] != 0 && buffer[4] != 1)) {
                return;  // Ignore invalid start1 values
            }
            
            memcpy(lastPackage, buffer, PACKET_SIZE); // Store last received packet
        
            height = buffer[1];  // Wave height
            speed = buffer[2];   // Wave speed
            start1 = buffer[3];  // Start wave strip 1
            start2 = buffer[4];  // Start wave strip 2
            reset_flag = buffer[5];  // Reset flag
            updateLCD(); // Update LCD with new values
            checkResetSignal();  // Procesar nuevos datos si llegan
        }
    }
}

void updateLCD() {
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