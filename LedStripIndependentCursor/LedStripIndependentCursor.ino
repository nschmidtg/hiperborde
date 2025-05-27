#include <FastLED.h>
#include <Arduino.h>
#include <LiquidCrystal.h>

// LED Configuration
#define NUM_LEDS 100 // Number of LEDs per strip
#define DATA_PIN_1 2
#define MAX_BRIGHTNESS 255  // Maximum brightness value
#define INITIAL_BRIGHTNESS 60
#define BRIGHTNESS_STEP 5   // How much to change brightness per button press

// Serial Protocol
#define SYNC_BYTE 255
#define RESET_BYTE 249
#define SILENCE_1_BYTE 250
#define PEAK_BYTE 251
#define SILENCE_2_BYTE 252
#define BREAK_BYTE 253
#define SILENCE_3_BYTE 254
#define PACKET_SIZE 6 // sync_byte + height + width + speed + start + phase

// Animation Constants
#define PHASE_CONTEMPLATIVE 0
#define PHASE_WAIT 1
#define PHASE_CHAOS 2
#define PHASE_BREATHING 3
#define PHASE_FADE_OUT 4
#define MAX_WAVES 10

// Breathing effect constants
#define BREATHING_TRANSITION_TIME 3000
#define FADE_OUT_TIME 5000  // 5 seconds fade out time

// Definiciones de botones
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int lcd_key     = 0;
int adc_key_in  = 0;

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
    uint8_t previousPhase = 0;
    uint8_t frameTime = 50; // default 50ms
    uint8_t waveSize = 33;
    uint8_t globalBrightness = INITIAL_BRIGHTNESS;
    int lastButtonState = btnNONE;
    unsigned long lastButtonPressTime = 0;
    uint8_t breathingState = 0;
    uint8_t currentBrightness = 0;
    unsigned long lastBreathingUpdate = 0;
    CRGB previousLeds[NUM_LEDS];
} state;

CRGB rgb(uint8_t red, uint8_t green, uint8_t blue) {
    // Convert RGB to GBR
    return CRGB(green, blue, red);
}


int read_LCD_buttons() {
  adc_key_in = analogRead(0);  // Lee A0

  // Asignamos rangos con tolerancia
  if (adc_key_in < 50)   return btnRIGHT;   // 0
  if (adc_key_in < 150)  return btnUP;      // 99
  if (adc_key_in < 350)  return btnDOWN;    // 256
  if (adc_key_in < 500)  return btnLEFT;    // 408
  if (adc_key_in < 700)  return btnSELECT;  // 639

  return btnNONE;         // 1023 (ningún botón presionado)
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
    for (int i = 0; i < MAX_WAVES; i++) {
        state.waves[i].active = false;
    }
    
    state.currentPhase = PHASE_CONTEMPLATIVE;
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
        
        // Store current phase before potential change
        state.previousPhase = state.currentPhase;
        
        // Handle phase changes based on buffer[5]
        switch (buffer[5]) {
            case RESET_BYTE:
                state.currentPhase = PHASE_CONTEMPLATIVE;
                break;
            case SILENCE_1_BYTE:
                state.currentPhase = PHASE_WAIT;
                break;
            case PEAK_BYTE:
                state.currentPhase = PHASE_CHAOS;
                break;
            case SILENCE_2_BYTE:
                state.currentPhase = PHASE_WAIT;
                break;
            case BREAK_BYTE:
                state.currentPhase = PHASE_BREATHING;
                break;
            case SILENCE_3_BYTE:
                state.currentPhase = PHASE_FADE_OUT;  // Changed to fade out
                break;
        }
        
        // Only initialize effects if phase actually changed
        if (state.previousPhase != state.currentPhase) {
            switch (state.currentPhase) {
                case PHASE_CONTEMPLATIVE:
                    resetAnimation();
                    break;
                case PHASE_BREATHING:
                    initializeBreathingEffect();
                    break;
                case PHASE_FADE_OUT:
                    initializeFadeOut();
                    break;
            }
        }
        
        updateLCD();
        newDataAvailable = false;
    }
} serial;


void showContemplativeEffect() {
    // Scale background color by global brightness
    uint8_t bgRed = (255 * state.globalBrightness) / MAX_BRIGHTNESS;
    uint8_t bgGreen = (127 * state.globalBrightness) / MAX_BRIGHTNESS;
    fill_solid(leds, NUM_LEDS, rgb(bgRed, bgGreen, 0));  // Warm orange background
    
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
            int brightness = state.height * (1.0 - positionFactor);  // Use raw height from serial
        
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
                leds[startPos + j] = rgb(255, 255, 255);
            }
        }
    }
    
    FastLED.show();
}

void initializeBreathingEffect() {
    state.breathingState = 0;
    state.currentBrightness = 0;
    state.lastBreathingUpdate = millis();
}

void showBreathingEffect() {
    // Clear all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - state.lastBreathingUpdate;
    
    // Get target brightness for current state
    uint8_t targetBrightness;
    switch (state.breathingState) {
        case 0:  // Start at 0
            targetBrightness = 0;
            break;
        case 1:  // Full brightness
            targetBrightness = state.globalBrightness;
            break;
        case 2:  // Half brightness
            targetBrightness = state.globalBrightness / 4;
            break;
        case 3:  // Full brightness again
            targetBrightness = state.globalBrightness;
            break;
        case 4:  // Half brightness again
            targetBrightness = state.globalBrightness / 2;
            break;
        case 5:  // Full brightness again
            targetBrightness = state.globalBrightness;
            break;
    }
    
    
    if (elapsed < BREATHING_TRANSITION_TIME) {
        // Calculate interpolation factor (0.0 to 1.0)
        float factor = (float)elapsed / BREATHING_TRANSITION_TIME;
        // Interpolate between current and target brightness
        state.currentBrightness = state.currentBrightness + (targetBrightness - state.currentBrightness) * factor;
    } else {
        // We've reached the target brightness
        state.currentBrightness = targetBrightness;
        // Move to next state
        state.breathingState = (state.breathingState + 1) % 6;
        state.lastBreathingUpdate = currentTime;
    }
    
    // Set all LEDs to the same color with interpolated brightness
    CRGB color = rgb(
        state.currentBrightness * 0.6,  // Reduced red component
        state.currentBrightness * 0.5,  // Increased green component
        0                               // No blue
    );
    
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void initializeFadeOut() {
    // Store current LED state before starting fade out
    for (int i = 0; i < NUM_LEDS; i++) {
        state.previousLeds[i] = leds[i];
    }
    state.phaseStartTime = millis();
}

void showFadeOutEffect() {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - state.phaseStartTime;
    
    if (elapsed < FADE_OUT_TIME) {
        // Calculate fade factor (1.0 to 0.0)
        float fadeFactor = 1.0 - ((float)elapsed / FADE_OUT_TIME);
        
        // Apply fade to all LEDs
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = state.previousLeds[i];
            leds[i].nscale8_video(fadeFactor * 255);  // Scale brightness by fade factor
        }
    } else {
        // Fade complete, set all LEDs to black
        fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    
    FastLED.show();
}

void setup() {
    delay(2000);  // Give USB time to stabilize
    Serial.begin(230400);
    while (!Serial);

    FastLED.addLeds<WS2811, DATA_PIN_1, GBR>(leds, NUM_LEDS);
    FastLED.setBrightness(MAX_BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    lcd.begin(16, 2);
    lcd.print("Hiperborde v2");
    
    state.phaseStartTime = millis();
}

void handleButtonPressed(int lcd_key) {
    unsigned long currentTime = millis();
    
    // Only process button if it's different from last state or enough time has passed
    if (lcd_key != state.lastButtonState || (currentTime - state.lastButtonPressTime) > 300) {
        if (lcd_key == btnUP) {
            // Increase brightness
            if (state.globalBrightness < MAX_BRIGHTNESS) {
                state.globalBrightness = min(state.globalBrightness + BRIGHTNESS_STEP, MAX_BRIGHTNESS);
                state.lastButtonPressTime = currentTime;
            }
        } else if (lcd_key == btnDOWN) {
            // Decrease brightness
            if (state.globalBrightness > 0) {
                state.globalBrightness = max(state.globalBrightness - BRIGHTNESS_STEP, 0);
                state.lastButtonPressTime = currentTime;
            }
        }
        
        // Update LCD with current brightness
        lcd.setCursor(0, 1);
        lcd.print("Bright: ");
        lcd.print(state.globalBrightness);
        lcd.print("   ");
        
        state.lastButtonState = lcd_key;
    }
}

void loop() {
    lcd_key = read_LCD_buttons();
    handleButtonPressed(lcd_key);  // Handle buttons independently of animation timing

    // Process all available packets immediately
    while (serial.readPacket()) {
        serial.processPacket();
    }

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
            case PHASE_BREATHING:
                showBreathingEffect();
                break;
            case PHASE_FADE_OUT:
                showFadeOutEffect();
                break;
        }
        
        state.lastFrameTime = currentTime;
    }
}