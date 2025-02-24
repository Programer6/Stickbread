# Stickbread

## Control and Temperature Monitoring System

This project is designed to allow the ESP32 microcontroller to control a stepper motor and carry out temperature monitoring using a Dallas Temperature sensor. It features a web interface that allows users to switch the motor on and off and view temperature readings in both Celsius and Fahrenheit.

## Features

- **Stepper Motor Control**: On/off control via a web interface to turn the pipe.
- **Temperature Measurement**: Temperature power measurements using the Dallas Temperature sensor to stop when cooking Stickbread.
- **Web Server Interface**: Connects to WiFi for remote access and control.

## Components Required

- ESP32 development board
- Stepper motor
- Motor driver (e.g., ULN2003)
- Dallas Temperature sensor (e.g., DS18B20)
- OneWire library for temperature sensor communication
- Jumper wires and breadboard for connections
