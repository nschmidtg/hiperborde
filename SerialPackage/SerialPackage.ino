#include <Arduino.h>
#include <LiquidCrystal.h>
#include <FastLED.h>

// LCD display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const int PACKET_SIZE = 6;  // Expected packet size
uint8_t buffer[PACKET_SIZE]; // Incoming data buffer
uint8_t lastPackage[PACKET_SIZE] = {0}; // Store the last received packet
uint8_t currentPackage[PACKET_SIZE] = {0}; // Store the last received packet
bool packetReady = false;    // Flag to indicate a complete packet
const byte SYNC_MAXMSP = 255;  // Sync byte for data from Max/MSP
const byte RESET_MAXMSP = 254;  // Reset byte for data from Max/MSP
// LED strip configuration
#define LED_PIN1     2          // First LED strip connected to pin 2
#define LED_PIN2     3          // Second LED strip connected to pin 3
#define NUM_LEDS     150        // 150 LEDs per strip
#define LEDS_PER_M   60         // 60 LEDs per meter
#define BRIGHTNESS 200      // General brightness (0-255)

// Wave parameters
byte height;                    // Max brightness of wave (0-255)
byte speed;                     // Speed in m/s (will be converted to pixels per frame)
byte start1;                    // Start new wave on strip 1 flag
byte start2;                    // Start new wave on strip 2 flag
byte reset_flag = 0;            // Reset flag

// Wave tracking
struct Wave {
    int position;                 // Center position of wave (in 16.16 fixed point)
    byte height;                  // Max brightness of this wave
    float speed;                  // Speed in pixels per frame
    bool active;                  // Whether this wave is active
};

#define MAX_WAVES 5             // Maximum number of simultaneous waves per strip
Wave waves1[MAX_WAVES];         // Waves for strip 1
Wave waves2[MAX_WAVES];         // Waves for strip 2

// LED data
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

// Timing
unsigned long lastFrameTime = 0;
const int frameDelay = 100;      // IMPORTANT!! increase if lcd screen is flickering.

void setup() {
    Serial.begin(115200);  // Start Serial

    lcd.begin(16, 2);
    lcd.clear();
    // Clear LED strips
    FastLED.addLeds<WS2811, LED_PIN1, GRB>(leds1, NUM_LEDS);
    FastLED.addLeds<WS2811, LED_PIN2, GRB>(leds2, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    delay(1000);
    lcd.print("Hiperborde");

    // Initialize LED strips
    

    
    

    // Initialize wave arrays
    for (int i = 0; i < MAX_WAVES; i++) {
        waves1[i].active = false;
        waves2[i].active = false;

    }
}


// Create a new wave on the specified array
void createWave(Wave* waves, byte waveHeight, byte waveSpeed) {
    // Find an inactive wave slot
    for (int i = 0; i < MAX_WAVES; i++) {
      if (!waves[i].active) {
        waves[i].position = 0;  // Start at beginning of strip
        waves[i].height = waveHeight;
        // Convert m/s to pixels per frame:
        // (m/s) * (LEDs per m) * (seconds per frame)
        waves[i].speed = (float)waveSpeed * LEDS_PER_M * (frameDelay / 1000.0);
        waves[i].active = true;
        return;
      }
    }
    // If we get here, all wave slots are in use
  }

  // Update all waves in the array and render to the LED strip
void updateWaves(Wave* waves, CRGB* leds) {
    // Clear the LED strip first
    FastLED.clear();
    
    // Process each active wave
    for (int w = 0; w < MAX_WAVES; w++) {
      if (waves[w].active) {
        // Update wave position (using fixed point math for smoother movement)
        waves[w].position += (int)(waves[w].speed * 256);
        
        // Check if wave has moved completely off the strip
        if ((waves[w].position >> 8) > (NUM_LEDS + 15)) {  // +15 for wave width
          waves[w].active = false;
          continue;
        }
        
        // Draw the wave - use a sine-like function for the wave shape
        int center = waves[w].position >> 8;  // Convert from 16.16 fixed point to integer
        
        for (int i = -15; i <= 15; i++) {  // Wave width of ~30 pixels
          int pos = center + i;
          if (pos >= 0 && pos < NUM_LEDS) {
            // Create a sine-like curve for brightness (using abs for simplicity)
            // This gives max brightness at center, fading to edges
            byte brightness = waves[w].height * (1.0 - (abs(i) / 15.0));
            
            // Add this brightness to existing LED value (for overlapping waves)
            leds[pos] = CRGB(0, brightness, 0);
          }
        }
      }
    }
  }

void processPackage() {
  if (packetReady && currentPackage[0] == SYNC_MAXMSP) {  // Ensure first byte is 255
    if (memcmp(lastPackage, currentPackage, PACKET_SIZE) == 0) {
        return;  // Ignore repeated packets
    }
    if((currentPackage[3] != 0 && currentPackage[3] != 1) || (currentPackage[4] != 0 && currentPackage[4] != 1)) {
        return;  // Ignore invalid start1 values
    }

    memcpy(lastPackage, currentPackage, PACKET_SIZE); // Store last received packet

    height = currentPackage[1];  // Wave height
    speed = currentPackage[2];   // Wave speed
    start1 = currentPackage[3];  // Start wave strip 1
    start2 = currentPackage[4];  // Start wave strip 2
    reset_flag = currentPackage[5];  // Reset flag

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("H:"); lcd.print(height);  // Wave height
    lcd.print(" S:"); lcd.print(speed); // Wave speed

    lcd.setCursor(0, 1);
    lcd.print("S1:"); lcd.print(start1); // Start wave strip 1
    lcd.print(" S2:"); lcd.print(start2); // Start wave strip 2

    if (reset_flag == RESET_MAXMSP) {  // Check reset byte
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reset");

        // Clear all waves
        for (int i = 0; i < MAX_WAVES; i++) {
            waves1[i].active = false;
            waves2[i].active = false;
        }
        // Clear LED strips
        fill_solid(leds1, NUM_LEDS, CRGB::Blue);
        fill_solid(leds2, NUM_LEDS, CRGB::Blue);
        FastLED.show();
        delay(1000);
        fill_solid(leds1, NUM_LEDS, CRGB::Black);
        fill_solid(leds2, NUM_LEDS, CRGB::Black);
        FastLED.show();
        delay(1000);
        FastLED.clear();
        noInterrupts();
        FastLED.show();
        interrupts();
        delay(1000);
    }

    // Create new waves based on start flags
    if (start1 == 1) {
        createWave(waves1, height, speed);
    }
    
    if (start2 == 1) {
        createWave(waves2, height, speed);
    }
    packetReady = false;  // Reset flag to receive new packets
  }
}
  

void loop() {
    processPackage(); // Process incoming data
    // Check for frame timing
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime >= frameDelay) {
        lastFrameTime = currentTime;
        
        // Update both LED strips
        updateWaves(waves1, leds1);
        updateWaves(waves2, leds2);
        
        FastLED.show();
    }
}

// This function is triggered when serial data arrives
void serialEvent() {
    if (Serial.available() >= PACKET_SIZE) {  // Ensure we have a full packet
        Serial.readBytes(buffer, PACKET_SIZE); // Read full packet
        if(buffer[0] == SYNC_MAXMSP) {  // Ensure first byte is 255
            memcpy(currentPackage, buffer, PACKET_SIZE); // Store last received packet
        }
        packetReady = true; // Mark packet as ready for processing
    }
}
