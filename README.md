# IoT-Based Rider Safety Intelligence Platform

An IoT-based smart helmet safety system designed to improve rider safety by detecting dangerous conditions such as **alcohol consumption, rider drowsiness, and crash impact**.  
The system uses an **ESP32 microcontroller** to continuously monitor sensors and trigger safety actions like **buzzer alerts and emergency notifications**.

---

# Project Overview

Road accidents involving two-wheelers often occur due to:

- Drunk driving
- Rider drowsiness
- Delayed emergency response after accidents

This project provides a **real-time monitoring system integrated with a helmet** to detect these conditions and respond automatically.

The system performs three main safety checks:

1. Alcohol Detection
2. Drowsiness Detection
3. Crash Detection

If any unsafe condition is detected, the system immediately triggers warnings or emergency alerts.

---

# System Architecture

Helmet Sensors  
│  
├── MQ-3 Alcohol Sensor  
├── IR Eye Blink Sensor  
└── MPU6050 Accelerometer  
│  
▼  
ESP32 Microcontroller  
│  
├── Decision Algorithms  
├── Safety Logic  
│  
▼  
Actions  
│  
├── Buzzer Warning  
├── Ignition Control  
└── Emergency Alert (SMS with GPS location)

---

# Hardware Components

| Component | Purpose |
|----------|---------|
| ESP32 | Main microcontroller |
| MQ-3 Alcohol Sensor | Detect alcohol in breath |
| IR Eye Blink Sensor | Detect eye closure (drowsiness) |
| MPU6050 | Detect crash impact |
| GPS Module | Get rider location |
| GSM Module | Send emergency SMS |
| Buzzer | Warning alert |

---

# Sensor Modules

## 1. MQ-3 Alcohol Sensor

### Description

The MQ-3 is a gas sensor designed specifically for detecting **ethanol vapors**.  
It uses a **tin dioxide (SnO₂) sensing layer** whose resistance changes when alcohol is present.

### Detection Range

0.05 mg/L – 10 mg/L alcohol concentration

### Output Type

| Pin | Output |
|----|------|
| A0 | Analog Output |
| D0 | Digital Output |

For this project **digital output (D0) is used**.

### Detection Logic

IF MQ3_D0 == HIGH  
→ Alcohol detected  
→ Activate buzzer  
→ Disable ignition  

ELSE  
→ System normal  

### Threshold Explanation

Typical analog readings:

| Condition | ADC Value |
|----------|-----------|
| Clean air | 800 – 1500 |
| Alcohol detected | 2000 – 3500 |

Approximate detection threshold: **2000 ADC value equivalent**

This prevents false detection from:

- perfumes
- smoke
- environmental gases

---

## 2. IR Eye Blink Sensor (Drowsiness Detection)

### Description

The IR blink sensor detects whether the rider’s eyes are **open or closed** using infrared reflection.

Components:

- IR LED (transmitter)
- Photodiode (receiver)

When eyes close, reflection changes and the sensor output changes.

### Why This Sensor is Used

- Low cost
- Low power consumption
- Compact size
- Works without camera processing

### Output Type

Digital Output

HIGH → eye closed  
LOW → eye open  

### Drowsiness Detection Algorithm

IF eye_closed  
→ start timer  

IF eye_closed_time > 2 seconds  
→ drowsiness detected  
→ activate buzzer  

### Threshold Justification

Human eye blink durations:

| Event | Duration |
|------|---------|
| Normal blink | 100–400 ms |
| Microsleep | >1 second |
| Dangerous drowsiness | >2 seconds |

Thus **2 seconds is used as the threshold**.

---

## 3. MPU6050 Crash Detection Sensor

### Description

The MPU6050 is a **6-axis Inertial Measurement Unit (IMU)** containing:

- 3-axis accelerometer
- 3-axis gyroscope

It measures:

Acceleration (Ax, Ay, Az)  
Angular velocity (Gx, Gy, Gz)

### Communication

I2C protocol

### Data Conversion

Acceleration values are converted using:

Acceleration (g) = raw_value / 16384

(for ±2g sensitivity mode)

### Crash Detection Algorithm

Step 1 – Calculate total acceleration

A_total = √(Ax² + Ay² + Az²)

Step 2 – Detect impact

IF A_total > 3g  
→ crash_possible

Step 3 – Confirm crash

IF crash_possible AND no movement  
→ crash_detected

### Threshold Justification

Typical acceleration levels for motorcycles:

| Event | Acceleration |
|------|-------------|
| Normal riding | 0.8g – 1.5g |
| Hard braking | 2g |
| Road bump | 2.5g |
| Crash impact | 3g – 10g |

Therefore **3g is used as the crash detection threshold**.

---

# Firmware Logic

The ESP32 continuously reads sensor data and applies decision algorithms.

System Flow:

START  

Initialize sensors  

Read alcohol sensor  
Read eye blink sensor  
Read accelerometer  

IF alcohol detected  
→ disable ignition  
→ activate buzzer  

IF drowsiness detected  
→ activate warning buzzer  

IF crash detected  
→ get GPS location  
→ send emergency SMS  

Repeat continuously

---

# Development Tools
---
Arduino IDE
ESP32 Board Support Package
Embedded C/C++
Serial Monitor for debugging

# Future Improvements
---
Possible improvements include:
AI-based camera drowsiness detection
Cloud monitoring dashboard
Mobile application integration
Real-time accident reporting system
Improved battery management

# References
---
MQ Sensors Library
https://github.com/miguel5612/MQSensorsLib

MPU6050 Arduino Library
https://github.com/jrowberg/i2cdevlib

IR Eye Blink Sensor Example
https://github.com/Arduinolibrary/IR-Eye-Blink-Sensor

Smart Helmet IoT Project Example
https://github.com/IoT-Projects-Ideas/Smart-Helmet-IOT

# Research Papers
---
Automatic Motorcycle Accident Detection Using Accelerometer and GPS
IEEE Intelligent Transportation Systems Conference

Driver Drowsiness Detection Using Eye Blink Monitoring
IEEE Transactions on Intelligent Transportation Systems

MQ-3 Alcohol Sensor Datasheet
Hanwei Electronics

---

# Authors

- **Manjit Bajaj**
- Github: https://github.com/Bajajmanjit12
  Firmware Developer  
  Department of Computer Engineering  
  Sanjivani College of Engineering, Kopargaon  

- **Anjali**  
  Hardware Lead  
  Responsible for sensor connections, module mounting, and ensuring stable hardware operation.

- **Shraddha**  
  Testing & Safety  
  Responsible for full system testing, assisting in mounting, conducting market research, and performing system simulations.

- **Arti**  
  Communication Lead  
  Responsible for GSM integration, SMS alert development, GPS handling, and minimizing false crash detection.

- **Nikita**  
  Dashboard Developer  
  Responsible for dashboard development (optional), displaying alert data, and supporting system testing and research.

---

# Collaborators

The following team members contributed to the development of this project:

- **Manjit Bajaj** – Firmware development, sensor integration, and system logic design  
- **Anjali** – Hardware setup, wiring, and sensor mounting  
- **Shraddha** – System testing, safety validation, and simulations  
- **Arti** – GSM communication, GPS integration, and emergency alert system  
- **Nikita** – Dashboard development and research support  

---

# Acknowledgements

We would like to thank the faculty members and mentors of  
**Sanjivani College of Engineering, Kopargaon** for their guidance and support in completing this project.

Special thanks to the open-source community for providing libraries, documentation, and development resources used in this project.

---
