#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

// LED Configuration
#define NUM_LEDS 100 // Number of LEDs per strip
#define DATA_PIN_1 2
#define BRIGHTNESS 255

// Serial Protocol
#define SYNC_BYTE 255
#define RESET_BYTE 254
#define PACKET_SIZE 6 // sync_byte + height + width + speed + start + restart

// Animation Constants
#define PHASE_CONTEMPLATIVE 50000  // 10 seconds
#define PHASE_WAIT 1000           // 3 seconds
#define PHASE_CHAOS 2000          // 5 seconds

#define MAX_WAVES 10  // Maximum number of concurrent waves

struct Wave {
    int position = 0;
    bool active = false;
    int waveSize = 33;
};

// Global Variables
CRGB leds[NUM_LEDS];
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

struct AnimationState {
    uint8_t height = 0;
    bool start = false;
    Wave waves[MAX_WAVES];
    unsigned long lastFrameTime = 0;
    unsigned long phaseStartTime = 0;
    uint8_t currentPhase = 0;
    uint8_t frameTime = 50; // default 50ms
    uint8_t waveSize = 33;
} state;

CRGB rgb(uint8_t red, uint8_t green, uint8_t blue) {
    // Convert RGB to GBR
    return CRGB(green, blue, red);
}

void addWave(Wave waves[]) {
    // Find first inactive wave slot
    for (int i = 0; i < MAX_WAVES; i++) {
        if (!waves[i].active) {
            waves[i].active = true;
            waves[i].position = 0;
            waves[i].waveSize = state.waveSize;
            break;
        }
    }
}

void resetAnimation() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reset");
    delay(100);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    // Reset all waves
    for (int i = 0; i < MAX_WAVES; i++) {
        state.waves[i].active = false;
    }
    
    state.currentPhase = 0;
    state.phaseStartTime = millis();
    

}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("H:");
    lcd.print(state.height);
    lcd.setCursor(6, 0);
    lcd.print("W:");
    lcd.print(state.waveSize);
    lcd.setCursor(0, 1);
    lcd.print("S:");
    lcd.print(state.start);
    lcd.setCursor(6, 1);
    lcd.print("F:");
    lcd.print(state.frameTime);
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
        if (buffer[1] > 254) return false; // height must be 0-254
        // if (buffer[2] < 5) return false; // width must be 5-40
        // if (buffer[2] > 40) return false; // width must be 5-40
        // if (buffer[3] > 120) return false; // frame rate must be 15-120
        // if (buffer[3] < 15) return false; // frame rate must be 15-120
        if (buffer[4] > 1) return false; // start must be 0 or 1
        return true;
    }

    void processPacket() {
        if (!newDataAvailable) return;
        
        state.height = buffer[1];
        state.waveSize = buffer[2];
        state.frameTime = buffer[3];
        state.start = buffer[4];
        
        if (buffer[5] == RESET_BYTE) {
            resetAnimation();
        }
        
        updateLCD();
        newDataAvailable = false;
    }
} serial;


void showContemplativeEffect() {
    // Now we can specify colors in RGB format
    fill_solid(leds, NUM_LEDS, rgb(0, 0, 0));  // Warm orange background: rgb(10, 5, 0)
    
    // Start new waves when impulse is received
    if (state.start) {
        addWave(state.waves);
    }
    
    // Update and render all active waves

    Wave* waves = state.waves;
    
    for (int w = 0; w < MAX_WAVES; w++) {
        if (!waves[w].active) continue;
        
        // Show wave effect
        for (int j = 0; j < waves[w].waveSize; j++) {
            int logicalIndex = waves[w].position - j;
            if (logicalIndex < 0 || logicalIndex >= NUM_LEDS) continue;
            
            int ledIndex = logicalIndex;
            float positionFactor = abs((waves[w].waveSize / 2.0) - j) / (waves[w].waveSize / 2.0); // 0 at peak, 1 at edge
            int brightness = state.height * (1.0 - positionFactor);
        
            // Define edge and peak colors
            CRGB edgeColor = rgb(48, 102, 91);   // Ocean Green
            CRGB peakColor = rgb(0, 42, 104);   // Ocean Blue
        
            // Interpolate between colors
            CRGB color = blend(peakColor, edgeColor, positionFactor * 255); // 0=peak, 255=edge
        
            // Apply brightness scaling
            color.nscale8_video(brightness);
        
            // Add the color to the current LED (accumulating effect)
            leds[ledIndex] += color;
        }
        
        // Advance wave position
        waves[w].position++;
        
        // Deactivate wave when it completes a cycle
        if (waves[w].position - waves[w].waveSize >= NUM_LEDS) {
            waves[w].active = false;
        }
    }
        
    
    FastLED.show();
}

void showChaosEffect() {
    fill_solid(leds, NUM_LEDS, rgb(0, 0, 0));  // Black
    
    const int maxFlashWidth = 5;
    const int numFlashes = random(2, 6);
    
    for (int i = 0; i < numFlashes; i++) {
        int startPos = random(NUM_LEDS - maxFlashWidth);
        int width = random(1, maxFlashWidth + 1);
        
        for (int j = 0; j < width; j++) {
            if (startPos + j < NUM_LEDS) {
                leds[startPos + j] = rgb(255, 255, 255);  // White
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
    FastLED.addLeds<WS2811, DATA_PIN_1, GBR>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    Serial.begin(230400);
    lcd.begin(16, 2);
    lcd.print("Hiperborde v2");
    
    state.phaseStartTime = millis();
}

void loop() {
    // Process all available packets immediately
    while (serial.readPacket()) {
        serial.processPacket();
    }
    
    // Update animation
    unsigned long currentTime = millis();
    if (currentTime - state.lastFrameTime >= state.frameTime) {
        updatePhase();
        
        switch (state.currentPhase) {
            case 0:
                showContemplativeEffect();
                break;
            case 1:
            case 3:
                fill_solid(leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
            case 2:
                showChaosEffect();
                break;
        }
        
        state.lastFrameTime = currentTime;
    }
}