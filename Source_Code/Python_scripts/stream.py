#drishtikon thermal feed receiver and saver

import socket
import numpy as np
import cv2
import os
import time 

# --- Configuration (Keep existing) ---
ESP32_IP = '192.168.152.147' 
TCP_PORT = 11000
IMAGE_WIDTH = 128 #also try with 32 or 128
IMAGE_HEIGHT = 96 #also try with 24 or 96
FRAME_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT * 4

SAVE_DIR = "thermal_dataset"
SAVE_TRIGGER_KEY = ord('s') 
SCALE_FACTOR = 5 

# --- New Function to receive all data reliably ---
def receive_all(sock, size):
    """Function to ensure we receive the exact 'size' bytes."""
    data = b''
    while len(data) < size:
        # Calculate how many more bytes are needed
        bytes_to_read = size - len(data)
        
        # Read a chunk of data
        chunk = sock.recv(bytes_to_read)
        
        # If no data is received, the connection is closed
        if not chunk:
            return None
        
        data += chunk
    return data

def receive_and_process():
    # 1. Setup Connection
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print(f"Connecting to {ESP32_IP}:{TCP_PORT}...")
    
    # Increase connect timeout for robustness
    s.settimeout(10.0) 
    try:
        s.connect((ESP32_IP, TCP_PORT))
    except socket.error as e:
        print(f"Connection failed: {e}")
        return # Exit the function on connection failure
        
    print("Connection established. Receiving frames...")

    # 2. Setup Dataset Folder
    if not os.path.exists(SAVE_DIR):
        os.makedirs(SAVE_DIR)
        print(f"Created save directory: {SAVE_DIR}")
        
    frame_counter = 0

    # 3. Main Data Loop
    # Set a timeout for receiving data inside the loop
    s.settimeout(5.0) 
    while True:
        try:
            # --- Network Data Acquisition (FIXED) ---
            # Use the custom function to reliably receive FRAME_SIZE bytes
            data = receive_all(s, FRAME_SIZE)

            if data is None:
                print("Server disconnected or timed out while receiving frame.")
                break 

            if len(data) != FRAME_SIZE:
                 # This check is mostly redundant if receive_all works, but good for safety
                print(f"Incomplete frame received: expected {FRAME_SIZE}, got {len(data)}") 
                break 

            # --- Data Processing ---
            thermal_data = np.frombuffer(data, dtype=np.float32)
            thermal_image = thermal_data.reshape((IMAGE_HEIGHT, IMAGE_WIDTH))
            
            # Normalization (creates norm_image)
            norm_image = cv2.normalize(thermal_image, None, 0, 255, cv2.NORM_MINMAX)
            norm_image = norm_image.astype(np.uint8)

            # --- INVERSION FIX (Kept from previous correction) ---
            norm_image_fixed = cv2.flip(norm_image, 0)
            
            # Color map and Scaling for Display 
            color_image = cv2.applyColorMap(norm_image_fixed, cv2.COLORMAP_JET)
            display_image = cv2.resize(
                color_image, 
                (IMAGE_WIDTH * SCALE_FACTOR, IMAGE_HEIGHT * SCALE_FACTOR), 
                interpolation=cv2.INTER_LINEAR
            )

            # --- Display and Key Check ---
            cv2.imshow('MLX90640 Interpolated Thermal Feed', display_image)
            key = cv2.waitKey(1) & 0xFF
            
            # --- Saving Logic ---
            if key == SAVE_TRIGGER_KEY: # Press 's' to save a frame
                frame_counter += 1
                file_name_gray = os.path.join(SAVE_DIR, f"gray_frame_{frame_counter:04d}.png")
                cv2.imwrite(file_name_gray, norm_image_fixed) 
                print(f"Saved frame {frame_counter} to {file_name_gray}")

            if key == ord('q'):
                break 
                
        except socket.timeout:
            print("Socket timed out while waiting for data. Continuing...")
            continue
        except socket.error as e:
            print(f"Socket error: {e}")
            break
        except Exception as e:
            print(f"An unexpected error occurred: {e}")
            break

    # 4. Cleanup 
    s.close()
    cv2.destroyAllWindows()
    print("Connection closed. Windows destroyed.")

if __name__ == '__main__':
    receive_and_process()