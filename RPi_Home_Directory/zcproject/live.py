# -*- coding: utf-8 -*-
import json
import numpy as np
import time
import threading
from PIL import Image
from picamera2 import Picamera2
from libcamera import Transform
from tensorflow.lite.python.interpreter import Interpreter
from flask import Flask, jsonify

app = Flask(__name__)

# --- Shared API state (updated by camera loop, read by Flask) ---
current_state = {"state": "starting", "confidence": 0.0, "fps": 0.0}

print("Loading configuration...")
with open('model_config.json', 'r') as f:
    config = json.load(f)
CLASSES = config['classes']
IMG_SIZE = tuple(config['img_size'])

print("Loading TFLite Model...")
interpreter = Interpreter(model_path="lane_model.tflite")
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print("Warming up camera...")
picam2 = Picamera2()
cam_config = picam2.create_still_configuration(
    main={"size": (640, 480)},
    transform=Transform(hflip=True, vflip=True)
)
picam2.configure(cam_config)
picam2.start()
time.sleep(2)

print("\n--- Autonomous Vision Core Initialized ---")

# --- Background Vision Worker ---
def vision_loop():
    global current_state
    print("Vision tracking loop engaged in background thread.")
    try:
        while True:
            start_time = time.time()

            # 1. Capture directly to RAM
            frame_array = picam2.capture_array()

            # 2. Preprocess directly from RAM
            img = Image.fromarray(frame_array).resize(IMG_SIZE)
            arr = np.array(img, dtype=np.float32) / 255.0
            arr = np.expand_dims(arr, axis=0)

            # 3. Predict
            interpreter.set_tensor(input_details[0]['index'], arr)
            interpreter.invoke()
            probs = interpreter.get_tensor(output_details[0]['index'])[0]

            # 4. Interpret results
            predicted_idx = int(np.argmax(probs))
            state = CLASSES[predicted_idx]
            confidence = float(probs[predicted_idx])

            # 5. Calculate FPS
            fps = 1.0 / (time.time() - start_time)
            
            # Print to terminal/systemd logs
            print(f"Vision: [{state:^10}] | Confidence: {confidence:.1%} | FPS: {fps:.1f}")

            # Safe update for the API endpoint
            current_state = {
                "state": state,
                "confidence": round(confidence, 3),
                "fps": round(fps, 1)
            }
    except Exception as e:
        print(f"Critical exception inside vision loop: {e}")
    finally:
        picam2.stop()

# --- API Endpoints ---
@app.route('/state', methods=['GET'])
def state():
    """Any connected code or device polls this endpoint to fetch latest lane data."""
    return jsonify(current_state)

# --- Launch Sequence ---
if __name__ == '__main__':
    # Automatically kick off the camera loop thread so it runs immediately on boot
    camera_thread = threading.Thread(target=vision_loop, daemon=True)
    camera_thread.start()
    
    print("Pi Lane Server running on http://0.0.0.0:5000")
    print("Network state monitoring available at: /state")
    app.run(host='0.0.0.0', port=5000, threaded=True)