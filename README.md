# 🚀 Stewart Platform Motion Simulator

A **3-DOF motion simulator** built with an ESP32 and three MG996R servos, driven by a custom Python racing game.

This project demonstrates how to map **virtual game motion (roll, pitch, height)** into **real mechanical movement** for an interactive motion platform.

---

## 🎥 Demonstration

### Joint Motion Test
<p align="center">
  <a href="https://github.com/user-attachments/XXXXX/video1.mp4">
    <img src="./assets/demo_thumbnail.png" width="540" alt="Joint Motion Demo">
  </a>
</p>
<div align="center">Click the thumbnail to play the joint motion test video</div>

---

### Final Output
<p align="center">
  <a href="https://github.com/user-attachments/XXXXX/video2.mp4">
    <img src="./assets/final_photo.jpg" width="540" alt="Final Motion Platform">
  </a>
</p>
<div align="center">Click the thumbnail to play the final demo video</div>

---

## 📸 Final Build

![Final Motion Platform](./assets/final_photo.jpg)

---

## 🔧 Project Description

This project consists of:

### 💻 Python Game (PC)
- Uses **pygame**
- Simulates road, car, trees, AI traffic
- Sends real-time roll, pitch, and height data over UART to ESP32

📂 Located in: `software/python-game/`

---

### ⚙️ ESP32 Firmware (ESP-IDF)
- Processes incoming motion data
- Applies smoothing
- Computes servo angles
- Drives servos using **LEDC PWM**
- Includes safety limits and soft motion

📂 Located in: `firmware/esp-idf/`

---

## 🛠️ Hardware

| Component | Quantity |
|-----------|----------|
| ESP32 WROOM 32 | 1 |
| MG996R Servos | 3 |
| 6V Servo Power Supply | 1 |
| Ball Joints & Linkages | — |
| 3D Printed Parts (STL Files) | Provided |

STL files are under `hardware/STL/`.

---

## 📡 Data Format (Python → ESP32)

The game sends data in this format via serial:

