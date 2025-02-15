#include <FastLED.h>

#define NUM_LEDS 200           // Número total de LEDs por tira
#define DATA_PIN_1 3           // Pin de datos para el primer par
#define DATA_PIN_2 6           // Pin de datos para el segundo par
#define BRIGHTNESS 200          // Brillo general (0-255)
#define RESET_COMMAND 30     // Comando de reinicio desde Max MSP
#define LED_BUILTIN 13


CRGB leds1[NUM_LEDS];         // Array para el primer par de tiras
CRGB leds2[NUM_LEDS];         // Array para el segundo par de tiras

unsigned long startTime = 0;   // Marca de tiempo para el ciclo
bool isOnPhase = true;         // Indica si estamos en la fase encendida
int phase = 0; // 0 apagado, 1 esperando, 2 caos


void setup() {
  FastLED.addLeds<WS2811, DATA_PIN_1, GRB>(leds1, NUM_LEDS);
  FastLED.addLeds<WS2811, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);          // Comunicación con Max MSP
  startTime = millis();        // Inicializa el tiempo del ciclo
}

void loop() {
  checkResetSignal();          // Verifica si se recibió un comando de reinicio

  unsigned long currentTime = millis();
  unsigned long elapsed = (currentTime - startTime) / 1000; // Tiempo transcurrido en segundos

  if (phase == 0) {
    if (elapsed < 10) {       // Fase encendida: 4 minutos (240 segundos)
      showContemplativeEffect();
    } else {
      phase = 1;       // Cambia a la fase apagada
      startTime = millis();    // Reinicia el temporizador para la fase apagada
    }
  } else if (phase == 1) {
    if (elapsed < 3) {       // Fase apagado: 3 segundos
      offAndWait();
    } else {
      phase = 2;       // Cambia a la fase apagada
      startTime = millis();    // Reinicia el temporizador para la fase apagada
    }
  } 
  else if (phase == 2) {
    if (elapsed < 5) {        
      showChaosEffect();
    } else {
      phase = 3;       
      startTime = millis();
    }
  }
   else if (phase == 3) {
    if (elapsed < 3) {       // Fase apagado: 3 segundos
      offAndWait();
    } else {
      phase = 0;       
      startTime = millis();
    }
  } 
}

void offAndWait() {
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(50);
}

void showContemplativeEffect() {
  const int waveSize = 20; // Número de LEDs en la ola
  const int maxBrightness = 200;
  const int minBrightness = 5;
  const int waveSpeed = 50;
  const int loopLength = NUM_LEDS + waveSize;
  bool stopLoop = false;

  for (int i = 0; i < NUM_LEDS; i++) {
    fill_solid(leds1, NUM_LEDS, CRGB(0, minBrightness, 0));
    fill_solid(leds2, NUM_LEDS, CRGB(0, minBrightness, 0));
    
    for (int j = 0; j < waveSize; j++) {
      int ledIndex = (i - j + NUM_LEDS) % NUM_LEDS;
      float positionFactor = abs((waveSize / 2.0) - j) / (waveSize / 2.0);
      int brightness = minBrightness + (maxBrightness - minBrightness) * (1.0 - positionFactor);
      leds1[ledIndex] = CRGB(0, brightness, 0);
      leds2[ledIndex] = CRGB(0, brightness, 0);
      if(checkResetSignal()) {
        stopLoop = true;
        break;
      }
    }
    if(stopLoop) {
      break;
    }
    
    FastLED.show();
    delay(waveSpeed);
  }
}


void showChaosEffect() {
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  int numSegments1 = random(2, 5); // Número aleatorio de segmentos para el primer par
  int numSegments2 = random(2, 5); // Número aleatorio de segmentos para el segundo par

  for (int j = 0; j < numSegments1; j++) {
    int startIndex = random(NUM_LEDS - 5); // Inicio aleatorio del segmento
    for (int k = 0; k < 5; k++) {
      leds1[startIndex + k] = CRGB::White; // Color aleatorio para el segmento
    }
  }

  for (int j = 0; j < numSegments2; j++) {
    int startIndex = random(NUM_LEDS - 5); // Inicio aleatorio del segmento
    for (int k = 0; k < 5; k++) {
      leds2[startIndex + k] = CRGB::White; // Color aleatorio para el segmento
    }
  }

  FastLED.show();
  delay(random(10, 80));       // Ajusta la velocidad de los destellos
}

bool checkResetSignal() {
  if (Serial.available() > 0) {
    int command = Serial.read();
    if (command == RESET_COMMAND) {
      startTime = millis();    // Reinicia el temporizador
      Serial.print(command);
      isOnPhase = true;        // Vuelve a la fase encendida
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);                      // wait for a second
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      Serial.print("adasd\n");
      return true;
    }
  }
  return false;
}

void delayWithReset(unsigned long duration) {
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  FastLED.show();
  unsigned long start = millis();
  while (millis() - start < duration) {
    if (checkResetSignal()) {
      break;
    }
    delay(100);
  }
}