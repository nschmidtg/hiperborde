#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

// LED Configuration
#define NUM_LEDS 200 // Number of LEDs per strip
#define DATA_PIN_1 2
#define BRIGHTNESS 100

// Serial Protocol
#define SYNC_BYTE 255
#define RESET_BYTE 254
#define PACKET_SIZE 6

// Animation Constants
#define WAVE_SIZE 33
#define WAVE_DURATION 100  // Total number of LEDs for the wave to complete its evolution
#define PHASE_CONTEMPLATIVE 50000  // 10 seconds
#define PHASE_WAIT 1000           // 3 seconds
#define PHASE_CHAOS 2000          // 5 seconds
#define FRAME_TIME 50             // 50ms between frames

#define MAX_WAVES 10  // Maximum number of concurrent waves

struct Wave {
    int position = 0;
    bool active = false;
};

// Global Variables
CRGB leds1[NUM_LEDS];
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

struct AnimationState {
    uint8_t height = 0;
    uint8_t speed = 0;
    bool start1 = false;
    Wave waves1[MAX_WAVES];  // Waves for strip 1
    unsigned long lastFrameTime = 0;
    unsigned long phaseStartTime = 0;
    uint8_t currentPhase = 0;
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
            break;
        }
    }
}

void resetAnimation() {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    // Reset all waves
    for (int i = 0; i < MAX_WAVES; i++) {
        state.waves1[i].active = false;
    }
    
    state.currentPhase = 0;
    state.phaseStartTime = millis(); 
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reset");
    delay(100);
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
        
        if (buffer[5] == RESET_BYTE) {
            resetAnimation();
        }
        
        updateLCD();
        newDataAvailable = false;
    }
} serial;


void showContemplativeEffect() {
    fill_solid(leds1, NUM_LEDS, rgb(0, 0, 0));
    
    // Start new waves when impulse is received
    if (state.start1) {
        addWave(state.waves1);
    }
    
    // Update and render all active waves
    Wave* waves = state.waves1;
    CRGB* leds = leds1;
    
    for (int w = 0; w < MAX_WAVES; w++) {
        if (!waves[w].active) continue;
        
        // Calculate wave progress (0 to WAVE_DURATION)
        int waveProgress = waves[w].position % WAVE_DURATION;
        float normalizedProgress = (float)waveProgress / WAVE_DURATION; // 0 to 1
        
        // Show wave effect
        for (int j = 0; j < WAVE_SIZE; j++) {
            int logicalIndex = waves[w].position - j;
            if (logicalIndex < 0 || logicalIndex >= NUM_LEDS) continue;
            
            int ledIndex = logicalIndex;
            float positionFactor = (float)j / WAVE_SIZE; // 0 at start, 1 at end of wave
            
            // Calculate wave shape using sine functions
            float waveShape = 0;
            
            if (normalizedProgress < 0.5) {
                // First half: wave builds up
                float buildProgress = normalizedProgress * 2; // 0 to 1
                float wavePosition = positionFactor * 2 * PI; // Convert to radians
                
                // Create a wave that starts flat and builds up
                waveShape = sin(wavePosition) * buildProgress;
                
                // Add a second harmonic for more natural look
                waveShape += sin(wavePosition * 2) * 0.3 * buildProgress;
                
                // Ensure wave starts from 0
                if (positionFactor < 0.5) {
                    waveShape *= (positionFactor * 2);
                }
            } else {
                // Second half: wave tears down
                float tearProgress = (normalizedProgress - 0.5) * 2; // 0 to 1
                float wavePosition = positionFactor * 2 * PI;
                
                // Create a wave that gradually flattens
                waveShape = sin(wavePosition) * (1 - tearProgress);
                
                // Add a second harmonic that fades faster
                waveShape += sin(wavePosition * 2) * 0.3 * (1 - tearProgress * 1.5);
                
                // Ensure wave ends at 0
                if (positionFactor > 0.5) {
                    waveShape *= (1 - (positionFactor - 0.5) * 2);
                }
            }
            
            // Convert wave shape to brightness (0 to 1)
            float brightnessFactor = (waveShape + 1) / 2; // Convert from -1..1 to 0..1
            int brightness = state.height * brightnessFactor;
            
            // Add slight randomization to brightness for more natural look
            brightness = max(0, min(255, brightness + random(-5, 5)));
            
            // Define edge and peak colors
            CRGB edgeColor = rgb(48, 102, 91);   // Ocean Green
            CRGB peakColor = rgb(0, 42, 104);   // Ocean Blue
            
            // Interpolate between colors based on wave position
            float colorBlend = abs(waveShape); // Use wave shape for color blending
            CRGB color = blend(peakColor, edgeColor, colorBlend * 255);
            
            // Apply brightness scaling
            color.nscale8_video(brightness);
            
            // Add the color to the current LED (accumulating effect)
            leds[ledIndex] += color;
        }
        
        // Advance wave position
        waves[w].position++;
        
        // Deactivate wave when it completes a cycle
        if (waves[w].position - WAVE_SIZE >= NUM_LEDS) {
            waves[w].active = false;
        }
    }
    
    FastLED.show();
}

void showChaosEffect() {
    fill_solid(leds1, NUM_LEDS, rgb(0, 0, 0));  // Black
    
    const int maxFlashWidth = 5;
    const int numFlashes = random(2, 6);
    
    for (int i = 0; i < numFlashes; i++) {
        int startPos = random(NUM_LEDS - maxFlashWidth);
        int width = random(1, maxFlashWidth + 1);
        
        for (int j = 0; j < width; j++) {
            if (startPos + j < NUM_LEDS) {
                leds1[startPos + j] = rgb(255, 255, 255);  // White
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
    FastLED.addLeds<WS2811, DATA_PIN_1, GBR>(leds1, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
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
    if (currentTime - state.lastFrameTime >= FRAME_TIME) {
        updatePhase();
        
        switch (state.currentPhase) {
            case 0:
                showContemplativeEffect();
                break;
            case 1:
            case 3:
                fill_solid(leds1, NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
            case 2:
                showChaosEffect();
                break;
        }
        
        state.lastFrameTime = currentTime;
    }
}