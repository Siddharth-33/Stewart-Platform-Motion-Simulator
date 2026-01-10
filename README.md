# 🚀 Stewart Platform Motion Simulator

A real-time **3-DOF Stewart Platform Motion Simulator** controlled by an ESP32 and driven by a custom Python racing game.

This project converts **virtual game physics (roll, pitch, height)** into **physical platform motion**, creating an immersive motion simulation experience using low-cost hardware.

---

## 📌 Project Highlights

✔ Real-time serial communication between Python game and ESP32  
✔ Smooth servo motion using LEDC PWM + filtering  
✔ Custom Stewart platform geometry  
✔ Safe mechanical limits and vibration reduction  
✔ Fully open-source hardware + software  
✔ Designed for learning robotics, control systems, and embedded programming  

---

## 🎯 Motivation

Most Stewart platforms are expensive and complex.  
This project was built to:

- Learn embedded systems with ESP32  
- Understand kinematics and motion mapping  
- Connect game physics to real hardware  
- Build an affordable motion simulator using common components  
- Create a visually impressive robotics project  

---

## 🎥 Demonstration

### 🔹 Joint Motion Test  
*(Link opens video)*

[▶ Click here to view Joint Motion Test](https://github.com/user-attachments/assets/0e61225e-309b-447a-a33e-9b52f0b40c71)

---

### 🔹 Final Motion Platform Output  

[▶ Click here to view Final Working Demo](https://github.com/user-attachments/assets/d23dc95c-ccee-44ce-9eca-fbe2ea60c7e9)

---

## 📸 Final Hardware Build

![Final Platform](./Stewart-Platform-Motion-Simulator/Video/Setup-Image.jpeg
)

---

## 🧠 System Overview
Python Game (PC)
|
| Serial UART (USB)
↓
ESP32 (ESP-IDF Firmware)
|
| LEDC PWM
↓
3 × MG996R Servos
|
↓
Stewart Platform Mechanics


---

## 🎮 Python Game

The Python game is built using **pygame** and simulates:

- Road perspective rendering
- Car movement
- Trees and AI vehicles
- Terrain height variation
- Vehicle roll, pitch and height

### Output Motion Data

The game computes:

- **Roll** → Car steering angle  
- **Pitch** → Terrain slope  
- **Height** → Vertical terrain displacement  

These values are sent in real-time to ESP32:
roll,pitch,height

Example:
0.124,-0.052,0.231


---

## ⚙️ ESP32 Firmware

Written using **ESP-IDF**.

Responsibilities:

- Receive serial data
- Apply smoothing filters
- Convert motion to servo angles
- Clamp safe limits
- Generate PWM using LEDC
- Move servos smoothly without vibration

---

## 🛠 Hardware Components

| Component | Quantity |
|---------|---------|
| ESP32 WROOM-32 | 1 |
| MG996R Servo Motors | 3 |
| External 6V Servo Power Supply | 1 |
| Ball Joints & Linkages | Multiple |
| 3D Printed Platform Parts | Yes |
| USB Cable | 1 |

---

## 🧩 Mechanical Design

The platform uses a **3-servo triangular Stewart configuration**:

- Servos at 120° spacing
- Linkages connected to top platform
- Inverted servo mounting for mechanical advantage
- Safe angle limits applied in software

All STL files are provided in: 
Stewart-Platform-Motion-Simulator/CAD_Design

---

## 📐 Motion Mapping

Each servo contributes to platform tilt based on geometry:
Servo1 = height + rollcos(0°) + pitchsin(0°)
Servo2 = height + rollcos(120°) + pitchsin(120°)
Servo3 = height + rollcos(240°) + pitchsin(240°)

These are scaled and added to a safe center angle.

---

## 🧹 Motion Smoothing

Two smoothing layers are applied:

### 1️⃣ Low-Pass Filter

Reduces noise from game:
filtered = previous + α * (input - previous)


### 2️⃣ Slew Rate Limiter

Limits servo speed:
change ≤ MAX_SERVO_STEP per cycle


This prevents vibration and mechanical stress.

---

## 📡 Communication

| Parameter | Value |
|--------|-------|
| Interface | UART over USB |
| Baud Rate | 115200 |
| Data Format | ASCII CSV |
| Update Rate | ~50 Hz |

---

---

## 🚀 How to Run

### ESP32

```bash
idf.py build
idf.py flash
idf.py monitor
```
Python Game
```bash
pip install pygame pyserial
python main.py
```
---

⚠ Safety Notes
---
Always power servos from external supply

Never power MG996R from ESP32 5V

Keep mechanical angle limits strict

Start with platform unloaded

🔮 Future Improvements
---

6-DOF Stewart platform

IMU feedback

PID closed loop control

VR headset integration

Wireless communication

Larger platform


👤 Author
---
Siddharth Mishra

Electronics & Robotics Enthusiast
Stewart Platform Motion Simulator Project




