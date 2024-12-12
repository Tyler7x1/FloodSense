# FloodSense

Real-Time Flood Monitoring System Web App

## Table of Contents

* [Overview](#overview)
* [Key Features](#key-features)
* [Hardware Requirements](#hardware-requirements)
* [Technical Requirements](#technical-requirements)
* [Installation and Setup](#installation-and-setup)
* [How It Works](#how-it-works)
* [Database Schema](#database-schema)
* [API Documentation](#api-documentation)
* [Socket.io Events](#socketio-events)
* [Contributing](#contributing)
* [License](#license)

## Overview

FloodSense is a Real-Time Flood Monitoring System Web App designed to provide communities with real-time flood data and alerts. It integrates IoT sensors and intuitive dashboards to empower users with actionable insights to prepare for and respond to flood events.

## Key Features

* Real-Time Monitoring: Collects live data from IoT sensors to monitor water levels and temperature.
* Custom Alerts: Users can configure personalized alerts to stay informed about specific flood conditions.
* Data Visualization: Provides interactive dashboards and geospatial maps for a comprehensive view of flood data.

## Hardware Requirements

* ESP32 Dev Module
* HC-SR04 Ultrasonic Sensor Module
* LM35 Temperature Sensor
* 16x2 LCD (I2C) Display
* LEDs lights
* Jumper Wires


## Technical Requirements

### Hardware

* IoT Sensors: Water level sensors, flow meters, and temperature sensors.
* Computing: Raspberry Pi or similar single-board computers.

### Software

* Backend: Node.js, Express.js
* Database: MongoDB
* Real-Time Communication: Socket.io

## Installation and Setup

### Prerequisites

* Node.js (v14 or higher)
* MongoDB (v4 or higher)
* npm (v6 or higher)

### Steps to Install

1. Clone the Repository:
```bash
git clone https://github.com/Tyler7x1/FloodSense.git
```
2. Navigate to the Project Directory:
```bash
cd FloodSense
```
3. Install Dependencies:
```bash
npm install
```
4. Set Up the Environment:
```bash
Rename .env.example to .env.
Add your MongoDB connection string and other environment variables.
```
5. Run the Setup Script:
```bash
npm run setup
```
6. Start the Server:
```bash
npm start
```

## How It Works

1. Data Collection: IoT sensors measure water levels, flow rates, and temperatures, sending this data to the server.
2. Data Storage: The server processes and stores sensor data in a MongoDB database.
3. Real-Time Communication: Using Socket.io, the server sends live updates to connected clients.
4. User Interaction: Users access data through a web dashboard with visualized insights and configurable alerts.

## Database Schema

The MongoDB database uses the following schema for flood data:
```json
{
  "temperature": {
    "type": "Number",
    "required": true
  },
  "distance": {
    "type": "Number",
    "required": true
  },
  "floodWarning": {
    "type": "Boolean",
    "required": true
  },
  "timestamp": {
    "type": "Date",
    "default": "Date.now"
  }
}
```

## API Documentation

### Endpoints

#### GET /

Description: Fetch the latest flood data.

Response:
```json
{
  "temperature": 25.5,
  "distance": 150.3,
  "floodWarning": false,
  "timestamp": "2024-12-12T08:00:00.000Z"
}
```

#### POST /data

Description: Add a new data entry.

Request Body:
```json
{
  "temperature": 26.1,
  "distance": 140.2,
  "floodWarning": true
}
```

#### GET /data

Description: Retrieve all flood data entries.

Response:
```json
[
  {
    "temperature": 25.5,
    "distance": 150.3,
    "floodWarning": false,
    "timestamp": "2024-12-12T08:00:00.000Z"
  },
  {
    "temperature": 26.1,
    "distance": 140.2,
    "floodWarning": true,
    "timestamp": "2024-12-12T09:00:00.000Z"
  }
]
```

## Socket.io Events

### newData

Description: Emits newly collected flood data to all connected clients.

Payload:
```json
{
  "temperature": 25.8,
  "distance": 145.6,
  "floodWarning": false
}
```

### disconnect

Description: Notifies the server when a client disconnects.

## Contributing

### Steps to Contribute

1. Fork the Repository:
```bash
git fork https://github.com/Tyler7x1/FloodSense.git
```
2. Create a New Branch:
```bash
git checkout -b feature/your-feature-name
```
3. Make Changes and Commit:
```bash
git commit -m "Add your feature description"
```
4. Push Changes:
```bash
git push origin feature/your-feature-name
```
5. Open a Pull Request via GitHub.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.