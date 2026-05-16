<h1 align="center">🏠 Smart Home Automation using ESP8266 & Hand Gesture Control</h1>

<p align="center">
A complete IoT-based Smart Home Automation System built using ESP8266 NodeMCU, relays, IR remote control, DHT11 sensor, and Sinric Pro cloud platform. The project enables users to control home appliances remotely through a mobile application, IR remote, Gesture control and manual switches while also monitoring room temperature and humidity in real time.
</p>

---

## 📌 Overview

This project is a Smart Home Automation System that allows users to control electrical appliances using:

- 📱 Mobile Application (Sinric Pro)
- 🎮 IR Remote Control
- ✋ Hand Gesture Control using Webcam
- 🔘 Manual Switches

The system also monitors room temperature and humidity using the DHT11 sensor and displays live status on an LCD display.

---

## ✨ Features

- WiFi Based Home Automation
- Mobile App Control using Sinric Pro
- Hand Gesture Control using Laptop Webcam
- IR Remote Appliance Control
- Manual Switch Control
- Real-Time Temperature Monitoring
- Real-Time Humidity Monitoring
- LCD Status Display
- Multi-Appliance Control
- WebSocket Communication
- AI-Based Finger Detection using MediaPipe
- Real-Time Relay Switching

---

## 🛠️ Hardware Components

- ESP8266 NodeMCU
- Relay Module
- DHT11 Sensor
- IR Receiver Module
- IR Remote
- 16x2 I2C LCD Display
- Push Buttons / Switches
- Jumper Wires
- Power Supply

---

## 💻 Software & Technologies

- Arduino IDE
- Python
- OpenCV
- MediaPipe
- WebSocket
- Sinric Pro
- Embedded C++
- IoT Technology

---

## ⚙️ Working Principle

- ESP8266 connects to WiFi network
- Sinric Pro enables remote control through mobile app
- IR receiver detects remote commands
- Manual switches allow local appliance control
- DHT11 measures temperature and humidity
- LCD displays live sensor data and relay status
- Laptop webcam captures hand gestures
- OpenCV and MediaPipe detect finger movements
- Gesture commands are sent to ESP8266 through WebSocket
- Relays switch appliances ON/OFF accordingly

---

## ✋ Hand Gesture Control

The project uses:

- OpenCV for webcam video processing
- MediaPipe for hand tracking
- Finger detection algorithm for gesture recognition
- WebSocket communication for real-time control

### Finger Mapping

- ☝️ Index Finger → Relay 1
- ✌️ Middle Finger → Relay 2
- 🤟 Ring Finger → Relay 3
- 🖐️ Pinky Finger → Relay 4

---

## 🔌 Pin Configuration

- Relay 1 → GPIO10
- Relay 2 → GPIO14
- Relay 3 → GPIO12
- DHT11 Sensor → GPIO2
- LCD SDA → GPIO4
- LCD SCL → GPIO5
- IR Receiver → GPIO16

---

## 🚀 How to Run

### ESP8266 Setup

- Install Arduino IDE
- Install ESP8266 Board Package
- Install required libraries
- Update WiFi and Sinric Pro credentials
- Upload Arduino code to ESP8266


### Python Gesture Control Setup

Install Python libraries:

````bash
pip install opencv-python mediapipe websocket-client
python mamatha.py
````

---

##  📟 LCD Display Output
- Temperature Display
- Humidity Display
- Relay ON/OFF Status

---

## 🎥 Project Demo

https://github.com/user-attachments/assets/e3b5fd78-a9ba-48bd-843e-51c556482eb3
