#include <Arduino.h>

#define RESET_COMMAND 30  // Command received from MaxMSP
#define LED_BUILTIN 13

unsigned long startTime;

bool checkResetSignal() {
    if (Serial.available() > 0) {
        int command = Serial.read();
        Serial.println(command);  // Corrected string formatting
        if (command == RESET_COMMAND) {
            startTime = millis();
            Serial.println("chao " + String(command));
            
            // Blink LED to show command was received
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            return true;
        }
    }
    return false;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(14400);
    startTime = millis();
}

void loop() {
    checkResetSignal();
    
    // Print elapsed time every second
    unsigned long currentTime = millis();
    unsigned long elapsed = (currentTime - startTime) / 1000;
    //Serial.println("Elapsed time: " + String(elapsed) + "s");
    
    //delay(1000);  // Prevents spamming the serial monitor
}
