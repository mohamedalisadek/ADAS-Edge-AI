# Advanced Driver Assistance System (ADAS) - Embedded Cyber-Physical Vehicle

## Overview

This project implements a complete Advanced Driver Assistance System (ADAS) on an autonomous four-wheel-drive vehicle platform. The system integrates real-time embedded control, machine learning-based lane detection, cloud telemetry, and a mobile dashboard to create a production-grade cyber-physical vehicle demonstrator.

The architecture combines:
- PIC16F877A microcontroller (20MHz) for safety-critical control
- Raspberry Pi 4B for perception and cloud connectivity
- TensorFlow Lite lane detection pipeline for edge inference
- Google Firebase Realtime Database for cloud telemetry
- Flutter mobile application for live monitoring

## Project Status

Final Project Report: Spring 2026, Zewail City University of Science and Technology  
Program: Communications and Information Engineering (CIE-349 / CIE-408)

Repository: https://github.com/MohammedAliSadek/ADAS-Embedded-System-Project  
Firebase Database: https://adas-driver-dashboard-default-rtdb.firebaseio.com/  
Model & Dataset: https://huggingface.co/MohammedAliSadek/adas-lane-detection

## System Architecture

### Three-Node Hardware Architecture

1. **PIC16F877A (Real-time Control Node)**
   - Runs all MCAL/HAL firmware with strict timing constraints
   - Handles four hardware interrupt sources
   - Drives motors and actuators with PWM control
   - Streams telemetry at 9600 baud to Raspberry Pi
   - Main loop frequency: 6.7 Hz with deterministic cycle time

2. **Raspberry Pi 4B (Perception and Connectivity Node)**
   - Runs two decoupled Python microservices (live.py and bridge.py)
   - Manages Pi Camera V2 with Picamera2 driver
   - Publishes lane detection results to Firebase
   - Forwards lane commands to PIC via UART
   - Vision pipeline independent of network I/O

3. **Firebase Realtime Database (Cloud Bus)**
   - Receives lane state and confidence from bridge.py
   - Receives full vehicle telemetry from PIC via UART parser
   - Provides sub-200ms round-trip latency for vehicle telemetry
   - Supports Flutter app real-time subscriptions

### Firmware Layered Architecture

```
Application (APP/main.c)
      ↓
Hardware Abstraction Layer (HAL/)
      ↓
Microcontroller Abstraction Layer (MCAL/)
      ↓
Services (STD_TYPES, BIT_MATH, MCU_CONFIG)
```

## Safety-Critical Features

### Feature 1: Forward Collision Warning (FCW) & Automatic Emergency Braking (AEB)

**Hardware:** HC-SR04 ultrasonic sensor (RA1 TRIG / RA2 ECHO)

**Distance Zones:**
- CLEAR (>50cm): Forward at 70% speed, buzzer off
- WARN (30-50cm): Forward at 40% speed, slow beep alert
- SLOW (10-30cm): Forward at 15% speed, fast beep alert
- STOP (≤10cm): Motor stop, continuous buzzer, emergency display

**Safety Features:**
- Sentinel error codes (400/401) for sensor faults
- Faulty/disconnected sensors treated as clear zone (fail-safe)
- Timer1 prescaler 1:8 provides 1.6µs/tick precision (36 ticks/cm)

### Feature 2: Automatic Door Safety Interlock

**Hardware:** Magnetic reed switch on RB4 (RBIF)

**Control Logic:**
- RBIF interrupt detects door state changes
- Immediate motor stop if door opens
- Red indicator LED on RB1 for visual feedback
- Safety gate: prevents motion until door is confirmed closed

**Implementation:**
- Hardware-assisted edge detection via RBIF
- PORTB pull-up resistors enabled for noise immunity
- Per-iteration safety check in main loop

### Feature 3: Emergency Stop with Toggle Interlock

**Hardware:** Tactile button on RB0 (falling-edge EXT INT)

**Control Logic:**
- Toggle-based operation: press once to activate, press again to clear
- Immediate motor cutoff on activation
- Audible warning (200ms beep) on state change
- UART notification to Raspberry Pi

### Feature 4: Automatic Headlight Control

**Hardware:** Light-dependent resistor (LDR) on AN0 (10-bit ADC)

**Three-State Machine:**
- LIT: Headlights off (ambient ADC < 362)
- WARNING: Dim headlights at 50% duty + single beep alert (362 < ADC < 562)
- DARK: Full headlights ON at 100% duty (ADC > 562)

**Hysteresis Implementation:**
- Symmetric dead-bands prevent flickering
- Transitions: LIT → WARNING → DARK (bidirectional)
- Satisfies two-output-level requirement (warning precedes control action)

## Hardware Components

| Component | Role | Interface |
|-----------|------|-----------|
| PIC16F877A (40-DIP) | Central MCU, 20MHz HS crystal | - |
| Raspberry Pi 4B | TFLite inference, Firebase bridge | UART / CSI |
| HC-SR04 Ultrasonic Sensor | Front obstacle distance | RA1 (TRIG), RA2 (ECHO) |
| LDR + Resistor | Ambient light sensor | AN0 (ADC Channel 0) |
| L298N Dual H-Bridge | 4WD motor driver | RD0-RD3, RC1, RC2 |
| Active Buzzer | Audible alert patterns | RC5 |
| PCF8574 I2C LCD | 16×2 HD44780 display | RC3 (SCL), RC4 (SDA) |
| Magnetic Reed Switch | Door-open detection | RB4 (RBIF) |
| Tactile Push Button | Emergency stop | RB0 (INT) |
| Pi Camera V2 | Road video capture | CSI connector |

## Raspberry Pi Software Pipeline

### Phase 1: Hardware Verification (cam_test.py)

Validates camera module in isolation before ML deployment:
- Initializes Picamera2 driver at 640×480 resolution
- Applies ISP-level hardware transform (flip corrections)
- Waits 2s for AEC/AWB convergence
- Captures test_shot.jpg as verification artifact

### Phase 2: In-Situ Dataset Engineering (collect_data.py)

Collects training data on exact vehicle conditions to eliminate domain shift:
- Three-class labeling taxonomy: safe, warning, off_track
- Captures frames with Unix epoch timestamps for temporal ordering
- Directory structure: lane_dataset/{class}/{class}_{timestamp}.jpg
- Prevents collision-free rapid capture and dataset analysis

### Phase 3: Edge Inference & Flask Microservice (live.py)

Runs TensorFlow Lite inference with zero-copy pipeline:
- DMA camera capture → numpy array (zero disk I/O)
- Image resize to 128×128 bilinear resampling
- Pixel scaling: [0,255] → [0.0,1.0] float32 normalization
- Quantized TFLite model inference with argmax decoding
- Flask endpoint /state (O(1), stateless query) returns current state + confidence
- Daemon threading ensures vision loop never blocks on network I/O

### Phase 4: Cloud Telemetry Bridge (bridge.py)

Polls live.py state and publishes to Firebase with smart caching:
- Polls http://localhost:5000/state at configurable intervals
- Writes only on genuine state transitions (reduces Firebase quota usage)
- Updates laneState, laneConfidence, lastUpdate to database root
- Handles network timeouts gracefully

### Phase 5: Model Training & HuggingFace Publication

Published model and dataset available for community use:
- Repository: https://huggingface.co/MohammedAliSadek/adas-lane-detection
- Enables reproducibility and domain knowledge sharing

## System Automation: systemd Daemonization

Two systemd services manage automated startup and recovery:

### lane.service (Vision Service)

Runs live.py continuous inference loop:
```
[Service]
Type=simple
ExecStart=/usr/bin/python3 /path/to/live.py
Restart=always
RestartSec=5
```

### firebase-bridge.service (Cloud Bridge Service)

Manages telemetry publication:
```
[Service]
Type=simple
ExecStart=/usr/bin/python3 /path/to/bridge.py
Restart=always
RestartSec=5
After=lane.service
```

### Boot Dependency Sequencing

- firebase-bridge.service waits for lane.service startup completion
- Ensures vision pipeline is operational before cloud polling begins
- Automatic restart on service failure with 5-second delay

## Pin Assignment (PIC16F877A)

| Pin | Label | Direction | Config | Function |
|-----|-------|-----------|--------|----------|
| 2 | RA0/AN0 | IN | Analog | LDR (ADC Channel 0) |
| 3 | RA1 | OUT | Digital | HC-SR04 TRIG |
| 4 | RA2 | IN | Digital | HC-SR04 ECHO |
| 19-22 | RD0-RD3 | OUT | Digital | L298N motor control (IN1-IN4) |
| 23 | RC4/SDA | I2C | Open-drain | LCD I2C data |
| 24 | RC5 | OUT | Digital | Buzzer (active HIGH) |
| 25 | RC6/TX | OUT | UART | UART TX to Raspberry Pi |
| 26 | RC7/RX | IN | UART | UART RX from Raspberry Pi |
| 27-30 | RD4-RD7 | OUT | Digital | Headlight LEDs 1-4 |
| 33 | RB0/INT | IN | INT | Emergency stop (falling-edge) |
| 34 | RB1 | OUT | Digital | Door 1 indicator LED |
| 37 | RB3 | OUT | Digital | Heartbeat/OK LED |
| 37 | RB4/RBIF | IN | RBIF | Door reed switch |
| 16-17 | RC1-RC2/CCP1-CCP2 | OUT | PWM | Motor ENA/ENB |
| 18 | RC3/SCL | I2C | Open-drain | LCD I2C clock |

## Interrupt Architecture

Single multiplexed ISR vector (__interrupt() isr()) handles:

1. **Timer0 Overflow** (~204.8µs, prescaler 1:4)
   - Drives BUZZER_Tick() for pattern generation

2. **EXT_INT / RB0 Falling Edge**
   - Toggles g_estop_active flag
   - Calls MOTOR_Stop() immediately

3. **UART RX (RCIF ∧ RCIE)**
   - Stores incoming lane command byte in g_lane_cmd

4. **RBIF (RB4 Edge)**
   - Door sensor ISR
   - Updates indicator LED
   - Triggers safety interlock

## Firebase Database Structure

```
/
├── laneState: "safe" | "warning" | "off_track"
├── laneConfidence: 0.0-1.0
├── lastUpdate: Unix timestamp
└── vehicles/
    └── car_01/
        ├── distance: cm
        ├── speed: %
        ├── buzzer_state: "OFF" | "SLOW_BEEP" | "FAST_BEEP" | "CONTINUOUS"
        ├── door_open: boolean
        ├── estop_active: boolean
        ├── ldr_value: 0-1023
        ├── headlight_state: "LIT" | "DIM" | "DARK"
        └── timestamp: Unix timestamp
```

## Mobile Application

Flutter-based dashboard for real-time vehicle monitoring:
- Live subscription to Firebase database
- Lane state visualization
- Telemetry display (distance, speed, door status)
- Emergency stop notification alerts
- Historical data logging (optional)

## Installation & Setup

### Hardware Prerequisites

- PIC16F877A microcontroller with 20MHz crystal oscillator
- Raspberry Pi 4B (minimum 2GB RAM recommended)
- All sensors and actuators listed in Bill of Materials
- USB-UART adapter for programming PIC (ICSP)
- Micro-SD card (≥32GB) for Raspberry Pi OS

### Raspberry Pi Setup

1. Install Raspberry Pi OS (Bullseye or later)
2. Enable Camera and UART via raspi-config
3. Install Python dependencies:
   ```bash
   sudo pip install -r RPi_Home_Directory/zcproject/requirements.txt
   ```
4. Configure Firebase credentials in environment variables (do NOT commit credentials to repository)
5. Enable systemd services for automatic startup

### PIC16F877A Firmware

1. Compile firmware using MPLAB X IDE or compatible toolchain
2. Program via ICSP interface using PICkit3 or equivalent
3. Verify pin assignments match hardware layout (Figure 2 in project report)

## Usage

### Starting the System

On Raspberry Pi:
```bash
sudo systemctl start lane.service
sudo systemctl start firebase-bridge.service
sudo systemctl status lane.service firebase-bridge.service
```

### Monitoring

Check service logs:
```bash
journalctl -u lane.service -f
journalctl -u firebase-bridge.service -f
```

### Camera Verification

Test camera hardware:
```bash
python3 cam_test.py
```

Captured image: test_shot.jpg

### Data Collection

Collect training data for model retraining:
```bash
python3 collect_data.py
```

Frames captured to: lane_dataset/{safe,warning,off_track}/

## Performance Characteristics

- **Main Loop Frequency:** 6.7 Hz (PIC16F877A)
- **Interrupt Latency:** <500µs (critical sections GIE-disabled)
- **Timer0 Precision:** ±204.8µs
- **UART Baud Rate:** 9600
- **Firebase Round-trip Latency:** <200ms
- **TFLite Inference Model:** Quantized (estimated <100ms per frame)
- **Camera Resolution:** 640×480 capture, 128×128 model input

## Literature & Standards Compliance

- **Forward Collision Warning:** ISO 22737, EuroNCAP FCW requirements (TTC ≤ 2s warning, TTC ≤ 1s AEB)
- **Embedded Safety Architectures:** IEC 61508, ISO 26262 hardware/software partitioning
- **Lane Detection:** Ultra-Fast Lane Detection (CNN-based approach adapted for embedded deployment)
- **IoT Telemetry:** Firebase Realtime Database for <200ms latency requirements

## Known Limitations & Future Work

1. **Testing:** Unit test suite for MCAL/HAL modules recommended
2. **Scalability:** Additional sensors would require external interrupt expander or higher-tier MCU
3. **Performance Profiling:** Formal WCET analysis and power consumption characterization pending
4. **Cloud Resilience:** Firebase failover and offline caching strategies to be documented

## Security Considerations

- Credentials are provided via environment variables; never commit secrets to repository
- UART communication is unencrypted (local CAN bus preferred for production)
- Firebase rules should enforce strict access control and authentication
- Regular security audits recommended for production deployments

## References

1. ISO 22737 - Forward Collision Warning Systems
2. EuroNCAP Safety Assessment - ADAS Requirements
3. Ultra-Fast Lane Detection (GitHub - turoad/laneNet)
4. IEC 61508 - Functional Safety of Electrical/Electronic Systems
5. ISO 26262 - Road Vehicles - Functional Safety

## License

Academic project for educational purposes at Zewail City University of Science and Technology.

## Contact

Project Author: Mohammed Ali Sadek  
Institution: Zewail City, University of Science and Technology  
Program: Communications and Information Engineering (CIE-349 / CIE-408)