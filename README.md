# RIDESENSE-X  
## IoT-Based Rider Safety Intelligence Platform

RIDESENSE-X is an IoT-based smart rider safety system designed to improve motorcycle safety using real-time monitoring, accident detection, drowsiness detection, alcohol detection, GPS tracking, and emergency alert systems.

The system uses an ESP32 microcontroller integrated with multiple sensors and communicates with a Flask-based web dashboard for live monitoring and emergency notifications.

---

# Features

- Real-time alcohol detection
- Rider drowsiness monitoring
- Crash and accident detection
- GPS-based live location tracking
- Telegram emergency alert system
- Real-time web dashboard monitoring
- Wi-Fi enabled IoT communication
- Sensor data visualization

---

# System Architecture

```text
Sensors
│
├── MQ-3 Alcohol Sensor
├── IR Drowsiness Sensor
├── MPU6050 Accelerometer & Gyroscope
└── GPS Module
        │
        ▼
ESP32 Microcontroller
        │
        ├── Sensor Processing
        ├── Decision Algorithms
        ├── Alert Logic
        │
        ▼
Wi-Fi Communication
        │
        ▼
Flask API Server
        │
        ├── Web Dashboard
        └── Telegram Alerts
```

---

# Hardware Components

| Component | Purpose |
|---|---|
| ESP32 Development Board | Main controller |
| MQ-3 Alcohol Sensor | Detect alcohol level |
| IR Sensor | Detect drowsiness |
| MPU6050 | Detect crash and movement |
| GPS Module (Neo-6M) | Live location tracking |
| Buzzer | Warning alerts |
| LED Indicators | Visual alerts |
| Breadboard & Jumper Wires | Circuit connections |

---

# ESP32 Pin Connections

| Component | ESP32 Pin |
|---|---|
| MQ-3 Sensor | GPIO34 |
| IR Sensor | GPIO27 |
| MPU6050 SDA | GPIO21 |
| MPU6050 SCL | GPIO22 |
| GPS TX | GPIO16 |
| GPS RX | GPIO17 |
| Buzzer | GPIO19 |

---

# Technologies Used

| Technology | Purpose |
|---|---|
| ESP32 | Embedded controller |
| Arduino IDE | Firmware programming |
| Python Flask | Backend server |
| HTML/CSS/JavaScript | Web dashboard |
| Chart.js | Graph visualization |
| Leaflet.js | GPS map display |
| Telegram Bot API | Emergency notifications |

---

# Working Logic

## 1. Alcohol Detection

- MQ-3 sensor continuously monitors alcohol concentration.
- If threshold exceeds safe limit:
  - Buzzer activates
  - Telegram alert is sent
  - Dashboard displays warning

---

## 2. Drowsiness Detection

- IR sensor monitors eye closure.
- If eyes remain closed for more than 2 seconds:
  - Drowsiness detected
  - Buzzer alert activated
  - Telegram warning sent

---

## 3. Crash Detection

- MPU6050 monitors acceleration and movement.
- Sudden impact detection followed by no movement confirms crash.
- GPS coordinates are captured.
- Emergency alert is sent through Telegram.

---
# Sensor Logic and Threshold Configuration

## 1. MQ-3 Alcohol Sensor

### Sensor Type
- Analog Output Used

### ESP32 Connection
| MQ-3 Pin | ESP32 Pin |
|---|---|
| A0 | GPIO34 |

### Working Logic

The MQ-3 sensor continuously measures alcohol concentration from the rider’s breath.

The ESP32 reads analog ADC values from the sensor and applies smoothing logic to reduce noise and false detection.

### Threshold Logic

```cpp
int alcoholThreshold = 1200;
```

### Detection Condition

```cpp
alcoholDetected = alcoholSmooth > alcoholThreshold;
```

### Typical ADC Values

| Condition | ADC Range |
|---|---|
| Clean Air | 300 – 900 |
| Mild Alcohol Presence | 1000 – 1500 |
| Strong Alcohol Detection | 1500 – 3000 |

### Logic Explanation

- If sensor value exceeds `1200`
  → Alcohol detected
  → Telegram alert sent
  → Dashboard warning activated

- Else
  → Rider considered safe

### Smoothing Formula

```cpp
alcoholSmooth = (alcoholSmooth * 0.8) + (alcoholRaw * 0.2);
```

This helps:
- Reduce noisy readings
- Prevent false triggering
- Stabilize sensor values

---

# 2. IR Drowsiness Sensor

### Sensor Type
- Digital Output Used

### ESP32 Connection

| IR Sensor Pin | ESP32 Pin |
|---|---|
| OUT | GPIO27 |

### Working Logic

The IR sensor monitors eye closure conditions.

### Sensor Output

| Sensor State | Output |
|---|---|
| Eye Open | HIGH |
| Eye Closed | LOW |

### Detection Logic

```cpp
bool isEyeClosed = (irState == LOW);
```

### Stability Logic

The system checks stable eye closure for multiple readings:

```cpp
if (stableCount >= 3)
```

This reduces:
- False detections
- Noise due to blinking

---

### Drowsiness Threshold

```cpp
const unsigned long DROWSY_TIME = 2000;
```

Threshold:
- 2000 ms = 2 seconds

### Detection Condition

```cpp
if (millis() - eyeClosedStart >= DROWSY_TIME)
```

### Logic Explanation

- Eye closed for less than 2 seconds
  → Normal blink

- Eye closed for more than 2 seconds
  → Drowsiness detected
  → Buzzer activated
  → Telegram alert sent

### Why 2 Seconds?

| Event | Duration |
|---|---|
| Normal Blink | 100–400 ms |
| Microsleep | >1 second |
| Dangerous Drowsiness | >2 seconds |

Thus 2 seconds is selected as safe detection threshold.

---

# 3. MPU6050 Crash Detection

### Sensor Type
- Digital I2C Communication

### ESP32 Connection

| MPU6050 Pin | ESP32 Pin |
|---|---|
| SDA | GPIO21 |
| SCL | GPIO22 |

### Working Logic

The MPU6050 continuously measures acceleration values along X, Y, and Z axes.

### Acceleration Calculation

```cpp
float currentAcc = sqrt(
  a.acceleration.x * a.acceleration.x +
  a.acceleration.y * a.acceleration.y +
  a.acceleration.z * a.acceleration.z
) / 9.81;
```

### Smoothed Acceleration

```cpp
float acc_g = (prevAcc * 0.7) + (currentAcc * 0.3);
```

### Impact Detection Threshold

```cpp
if (diff > 1.5)
```

### Crash Confirmation Threshold

```cpp
if (acc_g < 1.2)
```

### No Motion Delay

```cpp
unsigned long noMotionTime = 4000;
```

Threshold:
- 4000 ms = 4 seconds

---

### Crash Detection Process

Step 1:
Sudden acceleration change detected

```cpp
diff > 1.5
```

Step 2:
System waits for no movement

```cpp
millis() - impactTime > noMotionTime
```

Step 3:
Low acceleration confirms crash

```cpp
acc_g < 1.2
```

### Logic Explanation

| Condition | Result |
|---|---|
| High acceleration change | Impact detected |
| No movement after impact | Crash confirmed |
| Normal movement resumes | False alarm |

This multi-stage logic reduces false crash detection caused by:
- Road bumps
- Sudden braking
- Speed breakers

---

# 4. GPS Module

### Communication Type
- UART Serial Communication

### ESP32 Connection

| GPS Pin | ESP32 Pin |
|---|---|
| TX | GPIO16 |
| RX | GPIO17 |

### Working Logic

The GPS module continuously sends:
- Latitude
- Longitude

The ESP32 extracts coordinates using TinyGPSPlus library.

### GPS Usage

- Real-time dashboard location
- Accident alert location sharing
- Google Maps integration

---

# 5. Buzzer Alert System

### Output Type
- Digital Output

### ESP32 Connection

| Buzzer Pin | ESP32 Pin |
|---|---|
| Positive | GPIO19 |

### Alert Conditions

| Event | Buzzer Action |
|---|---|
| Alcohol detected | Warning tone |
| Drowsiness detected | Continuous beep |
| Crash detected | Emergency alert |

---

# Detection Summary Table

| Module | Output Type | Threshold |
|---|---|---|
| MQ-3 | Analog | >1200 ADC |
| IR Sensor | Digital | Eye closed >2 sec |
| MPU6050 | I2C Digital | diff >1.5 and acc <1.2 |
| GPS | UART Serial | Valid GPS coordinates |

# Repository Structure

```text
RIDESENSE-X/
│
├── firmware/
│   └── ridesense_x.ino
│
├── backend/
│   ├── app.py
│   └── requirements.txt
│
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── hardware/
│   ├── circuit_diagram.png
│   └── prototype.jpg
│
├── screenshots/
│   ├── dashboard.png
│   ├── alerts.png
│   └── gps_tracking.png
│
└── README.md
```

---

# Installation and Setup

## Step 1 — Clone Repository

```bash
git clone https://github.com/YOUR_USERNAME/RIDESENSE-X.git
cd RIDESENSE-X
```

---

# Backend Setup (Flask Server)

## Step 2 — Install Python Requirements

```bash
pip install -r requirements.txt
```

---

## Step 3 — Run Flask Server

Go to backend folder:

```bash
cd backend
python app.py
```

Output:

```text
Running on http://192.168.X.X:5000
```

Copy this IP address.

Example:

```text
http://192.168.215.196:5000
```

---

# ESP32 Firmware Configuration

## Step 4 — Update ESP32 Code

Open:

```text
firmware/ridesense_x.ino
```

Update:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

Then replace Flask server IP:

```cpp
String url = "http://192.168.X.X:5000/update";
```

and

```cpp
http.begin("http://192.168.X.X:5000/data");
```

with your actual IP address.

---

# Dashboard Configuration

## Step 5 — Update Dashboard Fetch URL

Open:

```text
frontend/index.html
```

Replace:

```javascript
fetch("http://192.168.X.X:5000/data")
```

with your Flask server IP.

Example:

```javascript
fetch("http://192.168.215.196:5000/data")
```

---

# Run Dashboard

## Step 6 — Open Dashboard

Open:

```text
frontend/index.html
```

in your browser.

The dashboard will display:

- Alcohol status
- Drowsiness status
- Crash alerts
- GPS location
- Real-time graphs

---

# Telegram Alert System

The system sends Telegram alerts during:

- Alcohol detection
- Drowsiness detection
- Accident detection

Accident alerts include:
- GPS coordinates
- Google Maps location link

---

# Sample Dashboard Output

Add screenshots here:

```text
screenshots/dashboard.png
screenshots/gps_tracking.png
screenshots/alerts.png
```

---

# Future Improvements

- AI camera-based drowsiness detection
- Cloud database integration
- Mobile application support
- Real-time analytics dashboard
- Helmet-mounted compact PCB
- Voice assistant integration

---

# Research References

## Libraries

- TinyGPSPlus
- Adafruit MPU6050
- Flask
- Chart.js
- Leaflet.js

## Research Papers

- Driver Drowsiness Detection Using Eye Monitoring
- IoT-Based Smart Helmet Safety System
- Motorcycle Accident Detection Using Accelerometer

---

# Authors

## Manjit Bajaj
Firmware Developer  
Department of Computer Engineering  
Sanjivani College of Engineering, Kopargaon

GitHub:  
https://github.com/Bajajmanjit12

---

## Team Members

| Name | Role |
|---|---|
| Manjit Bajaj | Firmware Development & System Integration |
| Anjali | Hardware Setup & Sensor Mounting |
| Shraddha | Testing & Safety Validation |
| Arti | GPS & Emergency Alert System |
| Nikita | Dashboard Development & Research |

---

# Acknowledgements

We thank the faculty members and mentors of  
Sanjivani College of Engineering, Kopargaon  
for their continuous guidance and support throughout this project.

Special thanks to the open-source community for providing libraries and technical resources.
