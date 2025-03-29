#!/bin/bash

# Set variables
# SKETCH_NAME="Simple/Simple.ino"  # Change this to your actual .ino file name
SKETCH_NAME="LedStripIndependent/LedStripIndependent.ino"  # Change this to your actual .ino file name
BOARD_FQBN="arduino:avr:mega"  # Fully Qualified Board Name for Arduino Mega
BAUD_RATE=460800               # Baud rate for serial communication

# Automatically detect the Arduino Mega port
PORT=$(arduino-cli board list | grep "Arduino Mega" | awk '{print $1}')

if [ -z "$PORT" ]; then
    echo "Error: No Arduino Mega detected. Please check the connection."
    exit 1
fi

echo "Arduino Mega detected on $PORT"

# Compile the sketch
echo "Compiling sketch..."
arduino-cli compile --fqbn $BOARD_FQBN $SKETCH_NAME --clean

if [ $? -ne 0 ]; then
    echo "Compilation failed. Check your code."
    exit 1
fi

# Upload the compiled code
echo "Uploading to Arduino Mega..."
arduino-cli upload --port $PORT --fqbn $BOARD_FQBN $SKETCH_NAME

if [ $? -ne 0 ]; then
    echo "Upload failed. Check your connection or code."
    exit 1
fi

echo "Upload successful!"

# Determine correct stty syntax (macOS vs Linux)
if [[ "$OSTYPE" == "darwin"* ]]; then
    STTY_CMD="stty -f $PORT $BAUD_RATE raw cs8 -cstopb -parenb"
else
    STTY_CMD="stty -F $PORT $BAUD_RATE raw -echo -cstopb -parenb"
fi

# Open serial monitor
echo "Opening Serial Monitor (Baud: $BAUD_RATE)..."
# eval $STTY_CMD
# cat $PORT
