# Smart Health Monitoring System

An IoT-based real-time health monitoring system using ESP32, Flask, and a web dashboard.

---

## Problem Statement

Traditional patient monitoring is periodic and manual, which may fail to detect sudden health deterioration.

This project provides continuous, real-time monitoring with automated alerts.

---

## Tech Stack

### Hardware
- ESP32
- MAX30102
- DS18B20
- LCD

### Software
- Backend: Python (Flask)
- Frontend: HTML, CSS, JavaScript, Chart.js
- Communication: HTTP, JSON, SMTP

---

## System Architecture

- ESP32 reads sensor data (Temperature, BPM, SpO2)
- Sends data via HTTP POST to Flask backend
- Flask:
  - Stores data
  - Detects abnormalities
  - Generates alerts
- Frontend:
  - Fetches `/get_data`
  - Displays real-time dashboard and graphs
- Email alerts sent via SMTP

---

## Features

- Real-time health monitoring
- Live charts using Chart.js
- Alert generation system
- Email notifications
- Multi-device dashboard access
- Historical data tracking

---

## Limitations

- SpO2 calculation is approximate
- No HTTPS (security improvement needed)
- No database (temporary storage only)

---

## Future Improvements

- Cloud deployment (AWS/GCP)
- Machine learning anomaly detection
- Mobile app integration
- Improved SpO2 accuracy

---

## How to Run

### Backend
```bash
pip install flask
python backend/app.py
```
### Frontend

Run the backend server, then open the dashboard in your browser:

http://localhost:5000

### ESP32

- Update WiFi credentials in the firmware code
- Update the server URL to match your system IP
- Upload the firmware to ESP32 using Arduino IDE / PlatformIO

## Authors
Divya Dharshini S G 
