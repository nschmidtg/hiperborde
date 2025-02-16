import numpy as np
import librosa
import librosa.display
import matplotlib.pyplot as plt
import csv

# Definir posiciones de los 10 parlantes en un espacio 2D (5 en cada muro opuesto)
positions_x = np.tile(np.linspace(0, 10, 5), 2)  # 5 parlantes por muro
positions_y = np.concatenate((np.zeros(5), np.ones(5) * 8))  # 0m en un muro, 8m en el otro
positions = np.column_stack((positions_x, positions_y))  # Coordenadas (x, y)

# Cargar los 10 audios
num_speakers = 10
sampling_rate = 44100  # Ajustar según los archivos
energy_data = []

audio_files = [f"./audios/mic{i+1}.wav" for i in range(num_speakers)]  # Nombres de archivos

audio_signals = []
for file in audio_files:
    y, sr = librosa.load(file, sr=sampling_rate)
    audio_signals.append(y)

# Asegurar que todas las señales tengan la misma longitud
min_length = min(len(y) for y in audio_signals)
audio_signals = [y[:min_length] for y in audio_signals]

# Calcular energía en ventanas de tiempo
frame_size = 1024
hop_size = 512
num_frames = min_length // hop_size

led_strip_data = []
num_leds = 100  # 100 LEDs por tira
led_positions = np.linspace(0, 10, num_leds)  # Posiciones interpoladas en la tira
speaker_positions = np.linspace(0, 10, 5)  # Posiciones de los parlantes

for frame in range(num_frames):
    frame_energies = np.array([np.sum(np.abs(y[frame * hop_size:(frame * hop_size) + frame_size])**2) 
                               for y in audio_signals])
    
    max_energy = np.max(frame_energies) if np.max(frame_energies) > 0 else 1  # Evitar división por 0
    normalized_energies = (frame_energies / max_energy) * 255  # Escalar a 0-255
    
    time_stamp = frame * (hop_size / sampling_rate)
    
    for i, led_x in enumerate(led_positions):
        distances = np.abs(speaker_positions - led_x)
        weights = np.exp(-distances)  # Peso exponencial para suavizar transición entre sectores
        weights /= np.sum(weights)  # Normalizar pesos
        
        intensity_left = int(np.dot(weights, normalized_energies[:5]))
        intensity_right = int(np.dot(weights, normalized_energies[5:]))
        
        color_R = intensity_left  # Rojo proporcional a intensidad
        color_G = 255 - intensity_left  # Verde inverso a intensidad
        color_B = 150  # Azul constante
        
        led_strip_data.append([time_stamp, "left", i, intensity_left, color_R, color_G, color_B])
        led_strip_data.append([time_stamp, "right", i, intensity_right, color_R, color_G, color_B])

# Guardar datos de LED strip para Arduino
with open("led_strip_data.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["time", "strip", "led_index", "intensity", "color_R", "color_G", "color_B"])
    for row in led_strip_data:
        writer.writerow(row)

print("Generación de datos de animación LED completada. Archivo guardado como led_strip_data.csv")

