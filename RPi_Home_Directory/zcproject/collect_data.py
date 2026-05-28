import time
import os
from picamera2 import Picamera2
from libcamera import Transform

base_path = "lane_dataset"
categories = ["safe", "warning", "off_track"]

for cat in categories:
    os.makedirs(os.path.join(base_path, cat), exist_ok=True)

print("Initializing Camera...")

picam2 = Picamera2()
config = picam2.create_still_configuration(
    main={"size": (640, 480)},
    transform=Transform(hflip=True, vflip=True) 
)
picam2.configure(config)
picam2.start()

time.sleep(2)

print("\n--- Lane Departure Warning Data Collection ---")
print("  's'  -> SAFE      (car fully between the lane lines)")
print("  'w'  -> WARNING   (wheel touching or crossing a line)")
print("  'o'  -> OFF TRACK (car fully outside the lane)")
print("  'q'  -> Quit\n")

counter = {"safe": 0, "warning": 0, "off_track": 0}

label_map = {
    "s": "safe",
    "w": "warning",
    "o": "off_track",
}

while True:
    totals = " | ".join([f"{k}: {v}" for k, v in counter.items()])
    print(f"  Counts -> {totals}")

    choice = input("Enter category (s/w/o/q): ").lower().strip()

    if choice == "q":
        print("\nFinal counts:")
        for k, v in counter.items():
            print(f"  {k}: {v} images")
        print("Exiting safely...")
        picam2.stop()
        break

    if choice not in label_map:
        print("  !! Invalid input, try again.")
        continue

    folder = label_map[choice]
    # Fixed timestamp to prevent overwriting images taken rapidly
    filename = f"{folder}_{time.time()}.jpg"
    filepath = os.path.join(base_path, folder, filename)

    picam2.capture_file(filepath)
    counter[folder] += 1
    print(f"  ✓ Captured {folder} #{counter[folder]}")
