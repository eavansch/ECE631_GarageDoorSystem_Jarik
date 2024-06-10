#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Define Wifi & MQTT values
const char* ssid = "ece631Lab";
const char* password = "esp32!IOT!";
const char* mqtt_server = "192.168.1.109";

// Connect to the client
WiFiClient espClient;
PubSubClient client(espClient);

// Define Pins
#define VOUT_PIN 36
//#define TRIG_PIN 23 
//#define ECHO_PIN 22

// Define values for PWM
//int PWM_FREQUENCY = 16;
//int PWM_CHANNEL = 0;
//int PWM_RESOLUTION = 8;
//int dutyCycle = 1;
int hallVal = 0;
int offset = 0;
char switchVal = '0';
int state = 1;
int RAGVal = 0;
StaticJsonDocument<200> doc;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

// Values to hold the echo times and if an echo has been received
unsigned long echoStartTime; 
unsigned long echoEndTime;   
//volatile boolean echoReceived = false; 

const int numReadings = 5; 
int readings[numReadings]; 
int readIndex = 0; 
unsigned long total = 0; 

// Setup wifi using example from Lab5
void setup_wifi() {
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
/*
void echoISR() {
  unsigned long now = micros();
  if (digitalRead(ECHO_PIN) == HIGH) {
    echoStartTime = now;
  } 
  else {
    echoEndTime = now;
    echoReceived = true;
  }
}
*/

void setup() {
  Serial.begin(115200);

  pinMode(VOUT_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  const int numSamples = 100; // Number of samples for averaging
  for (int i = 0; i < numSamples; i++) {
    offset += analogRead(VOUT_PIN);
    delay(10); // Delay between readings to reduce noise
  }
  offset /= numSamples;

  Serial.print("Magnetic Offset: ");
  Serial.println(offset);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  

  static unsigned long lastEchoTime = 0;

  // Reading Hall Effect value

  //switchVal = (char*) malloc(sizeof(char)+1);
  hallVal = analogRead(VOUT_PIN) - offset;

   total = total - readings[readIndex];

   readings[readIndex] = hallVal;
    
   total = total + readings[readIndex];
    // Increment readIndex for the next reading
   readIndex = (readIndex + 1) % numReadings;


  // Publish Hall Effect value with MQTT every second
  if (millis() - lastEchoTime > 1000) {
    lastEchoTime = millis();

  Serial.print("Hall Effect Value: ");
  Serial.println(hallVal);

  if(hallVal <= 50)
  {
    //LEDS TURN GREEN
    //SEND GREEN TO RAG
    RAGVal = 0;
  }

  else if(hallVal > 50 && hallVal <= 500)
  {
    RAGVal = 1;

  }
  else if(hallVal > 500)
  {
    //LEDS TURN RED
    //SEND RED TO RAG
    RAGVal = 2;
  }
    // State machine to determine the color of the light


    if (client.connected()) {
      char payload[50];
      sprintf(payload, "{\"RAG2\":\"%d\"}", RAGVal);
      client.publish("ece631/FinalProject/RAG2", payload);
      client.publish("ece631/FinalProject/MQTTSERVER", payload);
    }



  }
}

