#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

// LED Configuration
#define NUM_LEDS 150
#define DATA_PIN_1 2
#define DATA_PIN_2 3
#define BRIGHTNESS 200

// Serial Protocol
#define SYNC_BYTE 255
#define RESET_BYTE 254
#define PACKET_SIZE 6

// Animation Constants
#define WAVE_SIZE 20
#define PHASE_CONTEMPLATIVE 10000  // 10 seconds
#define PHASE_WAIT 3000           // 3 seconds
#define PHASE_CHAOS 5000          // 5 seconds
#define FRAME_TIME 50             // 50ms between frames

// Global Variables
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

struct AnimationState {
    uint8_t height = 0;
    uint8_t speed = 0;
    bool start1 = false;
    bool start2 = false;
    int wavePosition = 0;
    unsigned long lastFrameTime = 0;
    unsigned long phaseStartTime = 0;
    uint8_t currentPhase = 0;
} state;

void resetAnimation() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    state.currentPhase = 0;
    state.wavePosition = 0;
    state.phaseStartTime = millis();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reset");
    delay(100); // Brief delay for visibility
}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("H:");
    lcd.print(state.height);
    lcd.print(" S:");
    lcd.print(state.speed);
    lcd.setCursor(0, 1);
    lcd.print("S1:");
    lcd.print(state.start1);
    lcd.print(" S2:");
    lcd.print(state.start2);
}

// Serial Communication
struct SerialProtocol {
    uint8_t buffer[PACKET_SIZE];
    bool newDataAvailable = false;

    bool readPacket() {
        if (Serial.available() < PACKET_SIZE) return false;
        
        // Look for sync byte
        while (Serial.available() >= PACKET_SIZE) {
            if (Serial.peek() == SYNC_BYTE) {
                Serial.readBytes(buffer, PACKET_SIZE);
                if (validatePacket()) {
                    newDataAvailable = true;
                    return true;
                }
            } else {
                Serial.read(); // Discard byte and continue
            }
        }
        return false;
    }

    bool validatePacket() {
        if (buffer[0] != SYNC_BYTE) return false;
        if (buffer[3] > 1 || buffer[4] > 1) return false; // start1/start2 must be 0 or 1
        return true;
    }

    void processPacket() {
        if (!newDataAvailable) return;
        
        // Upscale height from 0-6 range to 0-255 range
        state.height = map(buffer[1], 0, 6, 0, 255);
        state.speed = buffer[2];
        state.start1 = buffer[3];
        state.start2 = buffer[4];
        
        if (buffer[5] == RESET_BYTE) {
            resetAnimation();
        }
        
        updateLCD();
        newDataAvailable = false;
    }
} serial;



void showContemplativeEffect() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    
    for (int j = 0; j < WAVE_SIZE; j++) {
        int ledIndex = (state.wavePosition - j + NUM_LEDS) % NUM_LEDS;
        float positionFactor = abs((WAVE_SIZE / 2.0) - j) / (WAVE_SIZE / 2.0);
        int brightness = state.height * (1.0 - positionFactor);
        
        if (state.start1) leds1[ledIndex] = CRGB(0, 255, 0);
        if (state.start2) leds2[ledIndex] = CRGB(0, 255, 0);
    }
    
    FastLED.show();
    state.wavePosition = (state.wavePosition + 1) % NUM_LEDS;
}

void showChaosEffect() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    
    const int maxFlashWidth = 5;
    const int numFlashes = random(2, 6);
    
    for (int i = 0; i < numFlashes; i++) {
        int startPos = random(NUM_LEDS - maxFlashWidth);
        int width = random(1, maxFlashWidth + 1);
        
        for (int j = 0; j < width; j++) {
            if (startPos + j < NUM_LEDS) {
                leds1[startPos + j] = CRGB::White;
                leds2[startPos + j] = CRGB::White;
            }
        }
    }
    
    FastLED.show();
}

void updatePhase() {
    unsigned long currentTime = millis();
    unsigned long phaseElapsed = currentTime - state.phaseStartTime;
    
    switch (state.currentPhase) {
        case 0: // Contemplative
            if (phaseElapsed >= PHASE_CONTEMPLATIVE) {
                state.currentPhase = 1;
                state.phaseStartTime = currentTime;
            }
            break;
            
        case 1: // Wait
            if (phaseElapsed >= PHASE_WAIT) {
                state.currentPhase = 2;
                state.phaseStartTime = currentTime;
            }
            break;
            
        case 2: // Chaos
            if (phaseElapsed >= PHASE_CHAOS) {
                state.currentPhase = 3;
                state.phaseStartTime = currentTime;
            }
            break;
            
        case 3: // Wait
            if (phaseElapsed >= PHASE_WAIT) {
                state.currentPhase = 0;
                state.phaseStartTime = currentTime;
            }
            break;
    }
}

void setup() {
    FastLED.addLeds<WS2811, DATA_PIN_1, GRB>(leds1, NUM_LEDS);
    FastLED.addLeds<WS2811, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    
    Serial.begin(115200);
    lcd.begin(16, 2);
    lcd.print("Hiperborde v2");
    
    state.phaseStartTime = millis();
}

void loop() {
    // Handle serial communication
    serial.readPacket();
    serial.processPacket();
    
    // Update animation
    unsigned long currentTime = millis();
    if (currentTime - state.lastFrameTime >= FRAME_TIME) {
        updatePhase();
        
        switch (state.currentPhase) {
            case 0:
                showContemplativeEffect();
                break;
            case 1:
            case 3:
                fill_solid(leds1, NUM_LEDS, CRGB::Black);
                fill_solid(leds2, NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
            case 2:
                showChaosEffect();
                break;
        }
        
        state.lastFrameTime = currentTime;
    }
}