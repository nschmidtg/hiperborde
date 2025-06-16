# Hiperborde LED Installation

An interactive LED installation that responds to real-time ocean wave data from Valparaíso's coast, creating dynamic wave patterns and visual effects on LED strips. The installation uses [marine weather](https://open-meteo.com/en/docs/marine-weather-api) data to drive the visual patterns, creating a direct connection between the ocean's state and the light installation.

## Project Information

This project is funded by the Ministerio de las Culturas, las artes y el patrimonio of Chile, through its Fondart Nacional, Línea Creación Artística - Nuevos Medios 2024. Folio 722614.

## Features

- Real-time ocean wave data visualization
- Multiple animation phases:
  - Contemplative: Smooth wave patterns with fade-in background
  - Chaos: Random flash patterns
  - Breathing: Pulsing light effect
  - Fade-out: Smooth transition to darkness
- LCD display for status and controls
- Adjustable brightness control via buttons
- Serial communication protocol for Max/MSP integration

## How It Works

The installation uses the Open-Meteo Marine Weather API to fetch real-time wave data from a specific point off the coast of Valparaíso (-33.035542, -71.601869). The system:

1. Fetches wave height and period data from the API
2. Maps the wave parameters to visual effects:
   - Wave height → Width and brightness of LED waves
   - Wave period → Timing of wave animations
3. Uses a state machine in Max/MSP to:
   - Trigger light phase changes
   - Synchronize with sound design
   - Manage the transition between different visual states

The Arduino receives this data through a custom serial protocol and translates it into dynamic light patterns, creating a real-time visualization of the ocean's state.

## Hardware Requirements

- Arduino Mega board
- LCD Keypad Shield (includes 16x2 LCD display and 5 buttons)
- 2 WS2811 LED strip
- 12V power supply (sufficient for LED strip ~10A)
- 1k Ohm resistor
- USB cable for programming and communication

## Software Requirements

- Required Arduino libraries:
  - FastLED
  - LiquidCrystal
- Max/MSP (for the control interface)
- avr-gcc toolchain (for compilation)
- avrdude (for uploading)
- curl (for API requests)

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/hiperborde.git
   cd hiperborde
   ```

2. Install required Arduino libraries using arduino-cli:
   ```bash
   # Install arduino-cli if not already installed
   brew install arduino-cli  # macOS
   # or
   sudo apt-get install arduino-cli  # Linux

   # Initialize arduino-cli
   arduino-cli core update-index
   arduino-cli core install arduino:avr

   # Install required libraries
   arduino-cli lib install "FastLED"
   arduino-cli lib install "LiquidCrystal"
   ```

3. Install avr-gcc and avrdude if not already installed:
   - On macOS (using Homebrew):
     ```bash
     brew install avr-gcc avrdude
     ```
   - On Linux:
     ```bash
     sudo apt-get install gcc-avr avr-libc avrdude
     ```

4. Make the upload script executable:
   ```bash
   chmod +x upload_and_monitor.sh
   ```

## Deployment

Instead of using the Arduino IDE for compilation and uploading (which can interfere with serial communication), we use the provided `upload_and_monitor.sh` script:

1. Connect your Arduino Mega to your computer via USB

2. Run the upload script:
   ```bash
   ./upload_and_monitor.sh
   ```

The script will:
- Compile the code using avr-gcc
- Upload the compiled code to the Arduino
- Start monitoring the serial output
- Handle any compilation or upload errors

## Hardware Setup

1. Connect the LCD Keypad Shield:
   - Simply plug the shield directly into the Arduino Mega
   - The shield includes:
     - 16x2 LCD display
     - 5 buttons (UP, DOWN, LEFT, RIGHT, SELECT)
     - All necessary connections are handled by the shield

2. Connect the LED strips and Arduino:
   - Arduino pin 2 to Protoboard with the resistor in serie
   - Led strips connected in parellel into the other end of the resistor.
   - VCC from led strips to 12V power supply
   - Arduino GND to power supply GND.

More details about hardware setup can be found on the document [Instrucciones de montaje.docx](Instrucciones%20de%20montaje.docx)


## Max/MSP Integration

The project includes a Max/MSP patch (`cerebro.maxpat`) that handles:
- Real-time API requests to Open-Meteo Marine Weather API
- Wave data processing and mapping
- Serial communication with Arduino
- State machine for light phase management
- Sound design
- Sync between sound and lights
- Control to trigger waves based on parameters from Open-Meteo Marine Weather API.

To use the Max/MSP interface:
1. Open `cerebro.maxpat` in Max/MSP
2. Start the audio engine
3. The patch will automatically:
   - Fetch wave data from the API
   - Process and map the data
   - Send commands to the Arduino
   - Manage light phase transitions
   - Start audio and sync with lightning

## Serial Protocol

The Arduino communicates with Max/MSP using a custom protocol:
- Packet size: 6 bytes
- Format: [sync_byte (255), height, width, speed, start_wave, phase]
- phase bytes:
  - 249: Reset
  - 250: Silence 1
  - 251: Peak (deactivated from patch)
  - 252: Silence 2
  - 253: Break (deactivated from patch)
  - 254: Silence 3 (deactivated from patch)

## Troubleshooting

1. If LEDs don't light up:
   - Check power supply voltage (should be 12V)
   - Verify data pin connection
   - Check for loose connections

2. If LCD doesn't display:
   - Verify the shield is properly seated on the Arduino
   - Check if the Arduino is receiving power
   - Try reseting the shield

3. If upload fails:
   - Check USB connection
   - Verify Arduino Mega is selected in the script
   - Check if avr-gcc and avrdude are properly installed
   - Try pressing the reset button on the Arduino before uploading

4. If API data isn't updating:
   - Check internet connection
   - Verify API endpoint is accessible
   - Check Max/MSP console for error messages


