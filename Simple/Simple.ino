#include <Arduino.h>

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
  }
  
  void loop() {
    
    // Check if data is available to read
    if (Serial.available() >= 5) {
      byte syncByte = Serial.read();
      Serial.println(syncByte);
      if (syncByte == 255) {
        Serial.println("***********************************");
      // Read two bytes
      byte byte1 = Serial.read();
      byte byte2 = Serial.read();
      byte byte3 = Serial.read();
      byte byte4 = Serial.read();
      // byte byte5 = Serial.read();
      // byte byte6 = Serial.read();
      // byte byte7 = Serial.read();
      // byte byte8 = Serial.read();
      
      // Print the values of the two bytes
      Serial.print("Byte 1: ");
      Serial.println(byte1);  // Print in hexadecimal format
      Serial.print("Byte 2: ");
      Serial.println(byte2);
      Serial.print("Byte 3: ");
      Serial.println(byte3);  // Print in hexadecimal format
      Serial.print("Byte 4: ");
      Serial.println(byte4);
      // Serial.print("Byte 5: ");
      // Serial.println(byte5);  // Print in hexadecimal format
      // Serial.print("Byte 6: ");
      // Serial.println(byte6);
      // Serial.print("Byte 7: ");
      // Serial.println(byte7);  // Print in hexadecimal format
      // Serial.print("Byte 8: ");
      // Serial.println(byte8);
      }  else {
        // If sync byte doesn't match, we ignore this byte and wait for the correct sync
        Serial.println("Sync byte mismatch, waiting for 0xFF...");
      }
    }
  }
  