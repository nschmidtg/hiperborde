#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

// LED Configuration
#define NUM_LEDS 100 // Number of LEDs per strip
#define DATA_PIN_1 2
#define DEFAULT_BRIGHTNESS 127

// LCD Button pins
#define BUTTON_UP 8
#define BUTTON_DOWN 9
#define BUTTON_LEFT 4
#define BUTTON_RIGHT 5

// Serial Protocol
#define SYNC_BYTE 255
#define RESET_BYTE 252
#define PEAK_BYTE 253
#define BREAK_BYTE 254
#define PACKET_SIZE 6 // sync_byte + height + width + speed + start + phase

// Animation Constants
#define PHASE_CONTEMPLATIVE 0
#define PHASE_WAIT 1
#define PHASE_CHAOS 2
#define MAX_WAVES 10  // Maximum number of concurrent waves

struct Wave {
    int position = 0;
    bool active = false;
    int waveSize = 33;
};

// Global Variables
CRGB leds[NUM_LEDS];
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
unsigned long lastLCDUpdate = 0;

struct AnimationState {
    uint8_t height = 0;
    bool start = false;
    Wave waves[MAX_WAVES];
    unsigned long lastFrameTime = 0;
    unsigned long phaseStartTime = 0;
    uint8_t currentPhase = 0;
    uint8_t frameTime = 50; // default 50ms
    uint8_t waveSize = 33;
    uint8_t maxBrightness = DEFAULT_BRIGHTNESS;  // Current max brightness
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
    
    state.currentPhase = PHASE_CONTEMPLATIVE;
}

// Function to scale a color by the current max brightness
CRGB scaleColor(CRGB color, uint8_t brightness) {
    // First scale by the input brightness (0-255)
    color.nscale8_video(brightness);
    // Then scale by the max brightness
    color.nscale8_video(state.maxBrightness);
    return color;
}

void handleButtons() {
    static unsigned long lastButtonPress = 0;
    const unsigned long debounceTime = 200; // 200ms debounce
    
    if (millis() - lastButtonPress < debounceTime) return;
    
    if (digitalRead(BUTTON_UP) == LOW) {
        state.maxBrightness = min(255, state.maxBrightness + 5);
        lastButtonPress = millis();
        updateLCD();
    }
    else if (digitalRead(BUTTON_DOWN) == LOW) {
        state.maxBrightness = max(0, state.maxBrightness - 5);
        lastButtonPress = millis();
        updateLCD();
    }
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
    lcd.print("B:");
    lcd.print(state.maxBrightness);
    lcd.setCursor(10, 1);
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
        
        // Handle phase changes based on buffer[5]
        switch (buffer[5]) {
            case RESET_BYTE:
                state.currentPhase = PHASE_CONTEMPLATIVE;
                resetAnimation();
                break;
            case PEAK_BYTE:
                state.currentPhase = PHASE_WAIT;
                break;
            case BREAK_BYTE:
                state.currentPhase = PHASE_CHAOS;
                break;
            // If buffer[5] is 0 or any other value, don't change phase
        }
        
        updateLCD();
        newDataAvailable = false;
    }
} serial;

void showContemplativeEffect() {
    fill_solid(leds, NUM_LEDS, rgb(0, 0, 0));
    
    if (state.start) {
        addWave(state.waves);
    }
    
    Wave* waves = state.waves;
    
    for (int w = 0; w < MAX_WAVES; w++) {
        if (!waves[w].active) continue;
        
        for (int j = 0; j < waves[w].waveSize; j++) {
            int logicalIndex = waves[w].position - j;
            if (logicalIndex < 0 || logicalIndex >= NUM_LEDS) continue;
            
            int ledIndex = logicalIndex;
            float positionFactor = abs((waves[w].waveSize / 2.0) - j) / (waves[w].waveSize / 2.0);
            int brightness = state.height * (1.0 - positionFactor);
            
            CRGB edgeColor = rgb(48, 102, 91);
            CRGB peakColor = rgb(0, 42, 104);
            
            CRGB color = blend(peakColor, edgeColor, positionFactor * 255);
            color = scaleColor(color, brightness);
            
            leds[ledIndex] += color;
        }
        
        waves[w].position++;
        
        if (waves[w].position - waves[w].waveSize >= NUM_LEDS) {
            waves[w].active = false;
        }
    }
    
    FastLED.show();
}

void showChaosEffect() {
    fill_solid(leds, NUM_LEDS, rgb(0, 0, 0));
    
    const int maxFlashWidth = 5;
    const int numFlashes = random(2, 6);
    
    for (int i = 0; i < numFlashes; i++) {
        int startPos = random(NUM_LEDS - maxFlashWidth);
        int width = random(1, maxFlashWidth + 1);
        
        // Calculate brightness based on width for more dynamic effect
        int baseBrightness = map(width, 1, maxFlashWidth, 200, 255);
        
        for (int j = 0; j < width; j++) {
            if (startPos + j < NUM_LEDS) {
                // Add some variation to brightness within each flash
                int brightness = baseBrightness + random(-20, 20);
                brightness = constrain(brightness, 0, 255);
                
                CRGB color = rgb(255, 255, 255);
                color = scaleColor(color, brightness);
                leds[startPos + j] = color;
            }
        }
    }
    
    FastLED.show();
}

void updatePhase() {
    // Phase changes are now handled in processPacket()
    // This function is kept for compatibility but doesn't do anything
}

void setup() {
    delay(2000);  // Give USB time to stabilize
    Serial.begin(230400);
    while (!Serial);

    // Setup button pins
    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT, INPUT_PULLUP);

    FastLED.addLeds<WS2811, DATA_PIN_1, GBR>(leds, NUM_LEDS);
    FastLED.setBrightness(255); // Set to max, we'll control brightness in software
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    lcd.begin(16, 2);
    lcd.print("Hiperborde v2");
    
    state.phaseStartTime = millis();
}

void loop() {
    // Process all available packets immediately
    while (serial.readPacket()) {
        serial.processPacket();
    }

    // Handle button presses
    handleButtons();

    // Show default LCD message if nothing arrives
    if (millis() - lastLCDUpdate > 1000 && !serial.newDataAvailable) {
        lcd.setCursor(0, 1);
        lcd.print("Waiting...");
        lastLCDUpdate = millis();
    }
    
    // Update animation
    unsigned long currentTime = millis();
    if (currentTime - state.lastFrameTime >= state.frameTime) {
        switch (state.currentPhase) {
            case PHASE_CONTEMPLATIVE:
                showContemplativeEffect();
                break;
            case PHASE_WAIT:
                fill_solid(leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
            case PHASE_CHAOS:
                showChaosEffect();
                break;
        }
        
        state.lastFrameTime = currentTime;
    }
}