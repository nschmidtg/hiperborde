#include <FastLED.h>

#define LED_PIN1 6  // Pin de datos para la primera tira
#define LED_PIN2 7  // Pin de datos para la segunda tira
#define NUM_LEDS 200  // Número total de LEDs por tira
#define SEPARATION 10.0  // Distancia en metros entre las tiras
#define STRIP_LENGTH 10.0 // Longitud de cada tira en metros

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

// Parámetros de las olas
float waveHeight = 5.5;  // 0..6 metros, define la altura de la ola
float wavePeriod = 5.0;  // 0..10 segundos, tiempo que tarda una ola en pasar completamente
int waveDirection = 45;  // 0..359 grados, dirección en la que viajan las olas

unsigned long previousMillis = 0;  // Almacena el tiempo del último frame
const float timeStep = 0.05;  // Intervalo de actualización de la simulación en segundos
float timeElapsed = 0;  // Tiempo total transcurrido en la simulación

void setup() {
    Serial.begin(115200);  // Inicializa la comunicación Serial
    FastLED.addLeds<WS2811, LED_PIN1, GRB>(leds1, NUM_LEDS);
    FastLED.addLeds<WS2811, LED_PIN2, GRB>(leds2, NUM_LEDS);
    FastLED.clear();
    FastLED.show();
}

void loop() {
    readSerialInput();  // Leer los parámetros desde Serial
    unsigned long currentMillis = millis();
    // Controla la frecuencia de actualización basada en timeStep
    if (currentMillis - previousMillis >= timeStep * 1000) {
        previousMillis = currentMillis;
        timeElapsed += timeStep;  // Avanza el tiempo de simulación
        renderWaves();
        FastLED.show();
        //printWaveSimulation();  // Imprimir animación en la terminal
    }
}

void renderWaves() {
    // Calcula la longitud de onda a partir del período
    float wavelength = wavePeriod * 1.5;  // Relación empírica para simular velocidad
    float k = TWO_PI / wavelength;  // Número de onda
    float w = TWO_PI / wavePeriod;  // Frecuencia angular
    float radDir = radians(waveDirection);  // Convierte dirección a radianes
    float cosDir = cos(radDir);
    float sinDir = sin(radDir);
    
    // Proyección de la ola dependiendo del ángulo
    // El ancho de la ola es más estrecho cuanto más alejada esté la dirección de 90°
    float projectedWavelength = wavelength / cosDir;  // Se ajusta según la dirección
    float projectedK = TWO_PI / projectedWavelength;  // Número de onda proyectado

    for (int i = 0; i < NUM_LEDS; i++) {
        float x = (i / (float)NUM_LEDS) * STRIP_LENGTH;
        
        // Calcula la altura de la ola en cada posición
        float waveBase = waveHeight * sin(projectedK * x - w * timeElapsed);
        float waveOffset = waveHeight * sin(projectedK * (x + SEPARATION * sinDir) - w * timeElapsed);
        
        leds1[i] = waveColor(waveBase);
        leds2[i] = waveColor(waveOffset);
    }
}





CRGB waveColor(float height) {
    // Mapea la altura de la ola a brillo y color
    uint8_t brightness = constrain(map(height * 100, 0, 600, 50, 255), 2, 255);
    
    // Genera un degradado de azul con tonos sutiles de cyan y blanco
    uint8_t blue = brightness;
    uint8_t green = brightness * 0.25;  // Agrega un toque de verde para cian
    uint8_t red = brightness * 0.15;    // Un toque de rojo para balancear
    
    return CRGB(red, blue, green);
}

void readSerialInput() {
    if (Serial.available() > 0) {
        // Leer datos de entrada desde el puerto serie
        waveHeight = Serial.parseFloat();  // Leer el valor flotante de la altura de la ola
        wavePeriod = Serial.parseFloat();  // Leer el valor flotante del periodo de la ola
        waveDirection = Serial.parseFloat(); // Leer el valor entero de la dirección de la ola

        // Restringir valores a los rangos definidos
        waveHeight = constrain(waveHeight, 0.0, 6.0);
        wavePeriod = constrain(wavePeriod, 0.1, 10.0);  // Evita división por cero
        waveDirection = constrain(waveDirection, 0, 359);
        
        Serial.print("Actualizado: waveHeight="); Serial.print(waveHeight);
        Serial.print(" wavePeriod="); Serial.print(wavePeriod);
        Serial.print(" waveDirection="); Serial.println(waveDirection);
    }
}

void printWaveSimulation() {
    Serial.print("\033[2J");  // Código ANSI para limpiar la consola
    Serial.print("\033[H");   // Código ANSI para mover el cursor al inicio
    Serial.println("\nVisualización de la ola (vista de plano paralelo):");
    
    // Proyección de la ola dependiendo del ángulo
    float radDir = radians(waveDirection);  // Convierte dirección a radianes
    float cosDir = cos(radDir);
    
    // Calcula la longitud de onda proyectada
    float wavelength = wavePeriod * 1.5;  // Relación empírica para simular velocidad
    float projectedWavelength = wavelength / cosDir;  // Ajusta longitud de onda según la dirección
    
    for (int i = 0; i < NUM_LEDS; i += 10) {  // Muestra solo algunos LEDs para simplificar
        float x = (i / (float)NUM_LEDS) * STRIP_LENGTH;
        
        // Calcula la ola para ambas tiras
        float wave1 = waveHeight * sin((TWO_PI / projectedWavelength) * x - (TWO_PI / wavePeriod) * timeElapsed);
        float wave2 = waveHeight * sin((TWO_PI / projectedWavelength) * (x + SEPARATION * sin(radians(waveDirection))) - (TWO_PI / wavePeriod) * timeElapsed);
        
        // Imprime la animación
        Serial.print("Tira 1: [");
        for (int j = 0; j < 10; j++) {
            if (j < map(wave1 * 100, 0, 600, 0, 10)) Serial.print("#");
            else Serial.print(" ");
        }
        Serial.print("]  Tira 2: [");
        for (int j = 0; j < 10; j++) {
            if (j < map(wave2 * 100, 0, 600, 0, 10)) Serial.print("#");
            else Serial.print(" ");
        }
        Serial.println("]");
    }
}



