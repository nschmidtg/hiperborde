# Hiperborde LED Installation

An interactive LED installation that responds to audio input through a Max/MSP interface, creating dynamic wave patterns and visual effects on LED strips.

## Features

- Real-time audio-reactive LED animations
- Multiple animation phases:
  - Contemplative: Smooth wave patterns with fade-in background
  - Chaos: Random flash patterns
  - Breathing: Pulsing light effect
  - Fade-out: Smooth transition to darkness
- LCD display for status and controls
- Adjustable brightness control via buttons
- Serial communication protocol for Max/MSP integration

## Hardware Requirements

- Arduino Mega board
- LCD Keypad Shield (includes 16x2 LCD display and 5 buttons)
- WS2811 LED strip (100 LEDs)
- 12V power supply (sufficient for LED strip)
- USB cable for programming and communication

## Software Requirements

- Arduino IDE (only for library management)
- Required Arduino libraries:
  - FastLED
  - LiquidCrystal
- Max/MSP (for the control interface)
- avr-gcc toolchain (for compilation)
- avrdude (for uploading)

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/hiperborde.git
   cd hiperborde
   ```

2. Install required Arduino libraries:
   - Open Arduino IDE
   - Go to Tools > Manage Libraries
   - Search and install:
     - FastLED
     - LiquidCrystal
   - Note: We only use Arduino IDE for library management, not for compilation or uploading

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

Note: If you get a permission error when trying to access the serial port, you may need to add your user to the `dialout` group (Linux) or install the appropriate USB drivers (macOS).

## Hardware Setup

1. Connect the LCD Keypad Shield:
   - Simply plug the shield directly into the Arduino Mega
   - The shield includes:
     - 16x2 LCD display
     - 5 buttons (UP, DOWN, LEFT, RIGHT, SELECT)
     - All necessary connections are handled by the shield

2. Connect the LED strip:
   - Data pin to Arduino pin 2
   - VCC to 12V power supply
   - GND to ground

## Max/MSP Integration

The project includes a Max/MSP patch (`hiperborde.maxpat`) that handles:
- Audio analysis
- Serial communication with Arduino
- Control interface for the installation

To use the Max/MSP interface:
1. Open `hiperborde.maxpat` in Max/MSP
2. Select the correct serial port in the patch
3. Start the audio engine
4. The patch will automatically communicate with the Arduino

## Serial Protocol

The Arduino communicates with Max/MSP using a custom protocol:
- Packet size: 6 bytes
- Format: [sync_byte, height, width, speed, start_wave, phase]
- Special bytes:
  - 255: Sync byte
  - 249: Reset
  - 250: Silence 1
  - 251: Peak
  - 252: Silence 2
  - 253: Break
  - 254: Silence 3

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


