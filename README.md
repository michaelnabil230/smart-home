# SmartHome Automation System

## Introduction

This project implements a monitoring and control system using an Arduino board with various components, including temperature sensors, LEDs, a servo motor, and an Ethernet module. The system allows remote monitoring and control through a web server hosted on the Arduino board.

## Components Used

- Arduino Board
- Ethernet Shield
- DHT11 Temperature and Humidity Sensor
- Servo Motor
- LEDs
- Gas Sensor (Analog)
- SD Card Module
- Resistors, wires, and other basic electronic components

## Libraries Used

- DHT (for DHT11 sensor)
- SPI (for SD card)
- Ethernet (for Ethernet connectivity)
- SD (for SD card operations)
- Servo (for servo motor control)

## Pin Configuration

- LEDs (Pin 34, 35, 36, 37)
- Servo Motor (Pin 22)
- Gas Sensor (Analog Pin A8)
- Temperature Sensors (Pin 30, 31, 32, 33)
- Other Devices (Fire Alarm, Water Pump, Fan - Pins 23, 24, 25)

## Initial Setup

1. Attach the servo motor to the designated pin (`servoMotor`).
2. Connect the fire alarm, water pump, fan, and gas sensor to their respective pins.
3. Ensure the correct pin configurations for LEDs (`led1` to `led4`).

## Running the System

1. Load the provided code onto the Arduino board.
2. Connect the Arduino to the Ethernet network.
3. Open a web browser and navigate to the assigned IP address (e.g., http://192.168.1.10).

## Web Server Functionality

- **LED Control:** Use the `/index.htm` page to control the LEDs.
- **Door Control:** Use the `/index.htm` page to open/close the servo-controlled door.
- **Temperature Monitoring:** Access temperature information using the `/tps.htm` pages.

## Gas Emergency System

- If gas concentration exceeds a threshold, the system activates the emergency response, turning on the fan, fire alarm, and indicating with an LED.

## Temperature Emergency System

- If any temperature sensor records a temperature exceeding the threshold, the emergency response is activated, turning on the fan, water pump, and fire alarm.

## Operation

1. The Arduino reads temperature values from four DHT11 sensors and checks for abnormal temperature conditions.
2. Gas levels are monitored using an analog gas sensor.
3. LEDs can be controlled through HTTP requests to the Arduino server.
4. The servo motor is used to simulate the opening and closing of a door.
5. The system provides a basic web interface for control and monitoring.

## Files

- `index.htm`: Main HTML file for controlling LEDs and servo.
- `tps.htm`: HTML file for displaying temperature information.

## Note

- Ensure that the required HTML files (`index.htm` and `tps.htm`) are present on the SD card.
- The project assumes a local network setup with a static IP address (`192.168.1.10` in the provided code). Adjust the IP address based on your network configuration.

## Author

[Michael Nabil](https://github.com/michaelnabil230)

Feel free to customize the code and adapt it to your specific requirements. For further assistance, refer to the Arduino documentation and the libraries used in the project.
