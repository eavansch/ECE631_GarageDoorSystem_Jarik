The purpose of this final lab assignment was a cumulative design of all previous labs in our course culminating in a garage door opener that includes NFC card authentication, UDS distance measures, Hall Effect sensor values, LED feedback, and MQTT communication tied to a Freeboard output. This was done utilizing the RPi4, Arduino, and HiveMQ platforms. Our group also included further developments in our Freeboard software displays as well as a physical 16x16 LED screen to display results of authentication as well as the opening/closing of our garage door.  In order to accomplish this, our group utilized all of the previous hardware and software from labs 3 through 8, and tied them together through our MQTT broker in order to achieve our final designed result. 

We used several pieces of hardware to configure and validate our final design and implementation which includes the following:
- 2 Raspberry Pi 4s (RPi4)
- 2 NodeMCU ESP32 Development boards (ESP32)
- ESP32 Onboard blue LED
- 2 SS49E Linear Hall-Effect Sensors (HES)
- 1 RCWL-1601 Ultrasonic Distance Sensor (UDS)
- 1 PN532 NFC/RFID MODULE V4 (PN532)
- 2 NFC Cards
- 1 32x16 Freetronics Dot-Matrix-Display (DMD)
- Macbook & Windows Laptops

The software utilized in our group’s final implementation is as follows:
- Arduino Integrated development environment (IDE)
- Linux operating system
- Nano editor for python
- Apple Terminal application
- PWM Libraries for ESP32
- HiveMQ MQTT broker
- MQTT Communication Protocol
- Freeboard dashboard
- MQTTool iOS Application

Erik's Hardware Diagram:
![erik hwd](https://github.com/eavansch/ECE631_GarageDoorSystem_Jarik/assets/89333755/60bab361-59ac-42d0-a810-7264e21dc9fd)

Jacob's Hardware Diagram:
![jhwd](https://github.com/eavansch/ECE631_GarageDoorSystem_Jarik/assets/89333755/66817ff9-1806-46ee-b196-e35fe6692fca)

Erik's Software Partition:
![erik software partition](https://github.com/eavansch/ECE631_GarageDoorSystem_Jarik/assets/89333755/17f65616-145b-407d-b83e-b6f15ee74591)

Freeboard validation:
![freeboard validation](https://github.com/eavansch/ECE631_GarageDoorSystem_Jarik/assets/89333755/58810f8b-4d90-4e9f-a44d-a0011e0b2377)

HiveMQ (MQTT) validation:
![mqttvalidation](https://github.com/eavansch/ECE631_GarageDoorSystem_Jarik/assets/89333755/79ef7a31-caac-42b3-b07c-6cff65267e60)
