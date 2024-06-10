/*  File: Spring24Final_Erik.ino
*   Author: Erik Van Schijndel 
*   Course: ECE 631 Spring 2024
*   Description: This sketch controls a hall effect sensor, a UDS, an onboard LED, and receives communication from our MQTT broker in order to model 
*   a garage door control setup.
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define TRIGPIN 23
#define ECHOPIN 22
#define WINDOW_SIZE_HALL 1
#define WINDOW_SIZE_UDS 5
#define LEDBLUE 2

// Constant parameters for our Lab network and MQTT server ip (RPI4)
const char* ssid = "ece631Lab"; 
const char* password = "esp32!IOT!";  
const char* mqtt_server = "192.168.1.125"; 

StaticJsonDocument<200> docUDS;
StaticJsonDocument<200> docHall;
StaticJsonDocument<200> docDoor;
// Initialize WiFi and MQTT client objects
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE_HALL  (50)
char msgHALL[MSG_BUFFER_SIZE_HALL];
#define MSG_BUFFER_SIZE_UDS (100)
char msgUDS[MSG_BUFFER_SIZE_UDS];
#define MSG_BUFFER_SIZE_GARAGE (50)
char msgGARAGE[MSG_BUFFER_SIZE_GARAGE];
// Given values for PWM testing and operation
int PWM_FREQUENCY = 16;
int PWM_CHANNEL = 0;
int PWM_RESOLUTION = 8;
int dutyCycle = 1;
//Timestamps for edges of echo signal
volatile unsigned long riseEdge;
volatile unsigned long fallEdge;
// Timers to track edge events 
unsigned long millisTimerHall;
unsigned long millisTimerUDS;
unsigned long LEDMillis;
bool LEDState = false;
// Array for our moving average filter
int measures[WINDOW_SIZE_UDS];
// calculated width of echo pulse 
volatile unsigned long pulseWidth;
// Calculated distance
float distance;
// Current time for signal processing
unsigned long currTime;
// Flag to store echo state
volatile int echoState;
long flashRate = 5000;
// Closed = 0; Closing = 1; Opening = 2; Opened = 3;
int garageState = 0;

int value = 0;
// Variables for implementing the moving average filter
int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE_HALL];
int HYSTERESIS = 0;

// Variables to assist in timing and sensor value calibration
long val = 0;
unsigned long millisTimer;
long offset;
int i = 0;


void callback(char* topic, byte* payload, unsigned int length)
{
  // Creating 200 byte document object to hold our MQTT content
  StaticJsonDocument<200> docInp;
  // Translate JSON 
  deserializeJson(docInp, payload, length);
  if(docInp.containsKey("NFC"))
  {
    String nfcAuth = docInp["NFC"].as<String>();
    if(nfcAuth == "Authorized" && garageState == 0)
    {
      docDoor["State"] = "OPENING";
      serializeJson(docDoor, msgGARAGE);
      client.publish("ece631/FinalProject/DoorState", msgGARAGE);
      flashRate = 1000;
      garageState = 2;
    }
    else if(nfcAuth == "Denied")
    {
    docDoor["State"] = "CLOSING";
    serializeJson(docDoor, msgGARAGE);
    client.publish("ece631/FinalProject/DoorState", msgGARAGE);
    flashRate = 1000;
    garageState = 1;
    }
  }
  else
  {
    String userText;
    for (unsigned int i = 0; i < length; i++) 
    {
      userText += (char)payload[i];
    }
    if(userText == "Close" && garageState == 3)
    {
      docDoor["State"] = "CLOSING";
      serializeJson(docDoor, msgGARAGE);
      client.publish("ece631/FinalProject/DoorState", msgGARAGE);
      garageState = 1;
    }
  }
}

// reconnect function attempts to connect to the MQTT server and subscribes to the "/Action/" topic
// If the connection fails, it retries every 5 seconds
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      client.subscribe("ece631/FinalProject/NFC");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ISR for echo pin state change
void echoInterrupt()
{
  // Save current time in microseconds
  currTime = micros();
  bool edge = digitalRead(ECHOPIN);
  // Detect rising edge of echo
  if(edge)
  {
    riseEdge = currTime;
  }
  // Detect falling edge of echo
  else
  {
    fallEdge = currTime;
    echoState = 1;
	// Determining pulse width
    pulseWidth = fallEdge - riseEdge; 
  }
}

// setup function initializes the serial communication, configures the sensor pin, connects to WiFi, and sets up MQTT
void setup() {
  // PWM setup for the trigger signal
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(TRIGPIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, dutyCycle);
  Serial.begin(115200);
  pinMode(36, INPUT);
  pinMode(ECHOPIN, INPUT);
  pinMode(LEDBLUE, OUTPUT);
  digitalWrite(LEDBLUE, LEDState);
  millisTimer = millis();
  millisTimerUDS = millis();
  echoState = 0;
  // Attach an interrupt to the echo pin to detect changes (rising/falling edge)
  attachInterrupt(digitalPinToInterrupt(ECHOPIN), echoInterrupt, CHANGE);
  for(i = 0; i < 100; i++) {
   offset += analogRead(36);
   delay(10);
  }
  offset = offset / i;
  WiFi.begin(ssid, password);
  // While not connected to lab wifi
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  // Validate connection and print local IP
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Connect to MQTT server with correct port
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  docDoor["State"] = "CLOSED";
  serializeJson(docDoor, msgGARAGE);
  client.publish("ece631/FinalProject/DoorState", msgGARAGE);
}
// The loop function contains the main logic for reading the sensor, applying the moving average filter, 
// determining the magnetic field orientation, and publishing the data via MQTT
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(echoState)
  {
	  // Sum of the last measurements for averaging
    static unsigned long sum = 0;
	  // Index for the current measurement in the array
    static int index = 0;
    // Update the sum and array with the latest measurement, wrap to zero if necessary.
    sum = sum + pulseWidth - measures[index];
	  measures[index] = pulseWidth;
    index = (index + 1) % WINDOW_SIZE_UDS;
    echoState = 0;
    // Every second, calculate the average distance and publish it
	if(millis() - millisTimerUDS >= 1000)
    {
	  // Calculate distance from microseconds to inches 
      distance = ((sum / float(WINDOW_SIZE_UDS)) * (0.0135039)) / float(2); // 343 m/s = 13400 inches / 10^6 seconds
      // Print distance result with 2 decimal points to serial for debugging
	    Serial.print(distance, 2);
      Serial.println(" inches");
      // Convert distance to String with two decimal places and then to C-style string
      String distanceStr = String(distance, 2);
	  // Prepare and publish the distance in JSON format over MQTT
      docUDS["Distance"] = distanceStr.c_str();
      docUDS["Units"] = "inches";
      serializeJson(docUDS, msgUDS);
      Serial.println(msgUDS);
      client.publish("ece631/FinalProject/Distance/SensorID/0", msgUDS);
      millisTimerUDS += 1000;
    }
  }

  if(millis() - millisTimer >= 1000)
  {
    millisTimer = millis();
  // Read sensor and adjust with calibrated offset
  val = analogRead(36) - offset;
  // Moving average filter calculations
  SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
  VALUE = val;     // Read the next sensor value
  READINGS[INDEX] = VALUE;           // Add the newest reading to the window
  SUM = SUM + VALUE;                 // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE_HALL;   // Increment the index, and wrap to 0 if it exceeds the window size

  HYSTERESIS = SUM / WINDOW_SIZE_HALL; 
  HYSTERESIS = HYSTERESIS;
  docHall.clear();
  docHall["Value"] = HYSTERESIS;
  // State machine to determine magnetic field orientation or absence based on HYSTERESIS value
    if(HYSTERESIS > 350 && garageState == 2) 
    {
      garageState = 3;
      docDoor["State"] = "OPEN";
      flashRate = 100;
      serializeJson(docDoor, msgGARAGE);
      client.publish("ece631/FinalProject/DoorState", msgGARAGE);
    } 
    else if(HYSTERESIS < 50 && garageState == 1)
    {
      garageState = 0;
      docDoor["State"] = "CLOSED";
      flashRate = 3000;
      serializeJson(docDoor, msgGARAGE);
      client.publish("ece631/FinalProject/DoorState", msgGARAGE);
    }
  serializeJson(docHall, msgHALL);
  client.publish("ece631/FinalProject/HallEffect/SensorID/0", msgHALL);
  }
    if (millis() - LEDMillis >= flashRate) 
    {
    LEDMillis = millis();
    LEDState = !LEDState;
    digitalWrite(LEDBLUE, LEDState);
    // Prepare the message to be sent via MQTT
    StaticJsonDocument<100> flashRateDoc;
    flashRateDoc["Flash Rate"] = flashRate;  // Store the flash rate in milliseconds
    char buffer[100];
    serializeJson(flashRateDoc, buffer);

    // Publish the flash rate
    client.publish("ece631/FinalProject/LED/Flashrate", buffer);
    }
}
