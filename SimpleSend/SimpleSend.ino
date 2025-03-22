#include <Arduino.h>
#include <LiquidCrystal.h>

const byte SYNC_ARDUINO = 255;  // Sync byte para datos enviados desde Arduino
const byte SYNC_MAXMSP  = 254;  // Sync byte para datos enviados desde Max/MSP
const byte ACK_BYTE = 253;  // Byte de confirmación
byte count;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  Serial.begin(115200);
  count = 0;
  lcd.begin(16, 2);
  lcd.print("SimpleSend");
}

void loop() {
  // 1. ENVIAR DATOS A MAX/MSP
  byte sensor1 = count % 253;  // Valores de prueba
  byte sensor2 = count + 1 % 253;
  byte checksum = (sensor1 + sensor2) & 255;  // Calcular checksum
  count++;
  Serial.write(SYNC_ARDUINO);  // Enviar sync byte (0xFF)
  Serial.write(sensor1);       // Enviar primer dato
  Serial.write(sensor2);       // Enviar segundo dato
  Serial.write(checksum);      // Enviar checksum
  

  delay(500);  // Simula muestreo de datos

  // 2. RECIBIR DATOS DESDE MAX/MSP
  if (Serial.available() >= 4) {  // Esperar al menos 3 bytes
    if (Serial.read() == SYNC_MAXMSP) {  // Si el primer byte es 0xFE
      byte cmd1 = Serial.read();  // Leer primer dato enviado por Max
      byte cmd2 = Serial.read();  // Leer segundo dato enviado por Max
      byte received_checksum = Serial.read();

      byte computed_checksum = (cmd1 + cmd2) & 255;
      if (received_checksum == computed_checksum) {  // Validar checksum

      // Hacer algo con los valores recibidos, por ejemplo, cambiar LEDs
        lcd.setCursor(0, 1);
        lcd.print(cmd1);
        lcd.print(" ");
        lcd.print(cmd2);

        // 3. ENVIAR CONFIRMACIÓN (ACK)
        Serial.write(ACK_BYTE);  // Enviar byte de confirmación
      }
    }
  }
}
