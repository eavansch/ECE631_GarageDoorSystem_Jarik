#include <DMD32.h>  //--> DMD32 by Qudor-Engineer (KHUDHUR ALFARHAN) : https://github.com/Qudor-Engineer/DMD32
#include "fonts/SystemFont5x7.h"
#include "fonts/Arial_black_16.h"
//----------------------------------------

// Fire up the DMD library as dmd.
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
#define VOUT_PIN 36
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// Timer setup.
// create a hardware timer  of ESP32.
hw_timer_t * timer = NULL;

//________________________________________________________________________________IRAM_ATTR triggerScan()
//  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning,
//  this gets called at the period set in Timer1.initialize();
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}

int hallVal = 0;
int offset = 0;
char switchVal = 'G';

// Values to hold the echo times and if an echo has been received
unsigned long echoStartTime; 
unsigned long echoEndTime;   
//volatile boolean echoReceived = false; 

const int numReadings = 5; 
int readings[numReadings]; 
int readIndex = 0; 
unsigned long total = 0; 


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}


void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("return the clock speed of the CPU.");
  // return the clock speed of the CPU.
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  delay(500);

  Serial.println();
  Serial.println("Timer Begin");
  // Use 1st timer of 4.
  // devide cpu clock speed on its speed value by MHz to get 1us for each signal  of the timer.
  timer = timerBegin(0, cpuClock, true);
  delay(500);

  Serial.println();
  Serial.println("Attach triggerScan function to our timer.");
  // Attach triggerScan function to our timer.
  timerAttachInterrupt(timer, &triggerScan, true);
  delay(500);

  Serial.println();
  Serial.println("Set alarm to call triggerScan function.");
  // Set alarm to call triggerScan function.
  // Repeat the alarm (third parameter).
  timerAlarmWrite(timer, 300, true);
  delay(500);

  Serial.println();
  Serial.println("Start an alarm.");
  // Start an alarm.
  timerAlarmEnable(timer);
  delay(500);

  pinMode(VOUT_PIN, INPUT);

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
  

  static unsigned long lastEchoTime = 0;

  dmd.selectFont(Arial_Black_16);

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

  switch(switchVal)
  {
    case 'R':
    if(hallVal <= 50)
    {
      switchVal = 'G';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"DRIVE", 5, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
    }
    else if(hallVal >= 50 && hallVal <= 500)
    {
      switchVal = 'A';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"SLOW", 4, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
    }
    break;

    case 'A':
    if(hallVal >= 500)
    {
      switchVal = 'R';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"STOP", 4, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
    }
    else if(hallVal <= 50)
    {
      switchVal = 'G';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"DRIVE", 5, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
    }
    break;

    case 'G':
    if(hallVal >= 500)
    {
      switchVal = 'R';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"STOP", 4, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
    }
    else if(hallVal >= 50 && hallVal <= 500)
    {
      switchVal = 'A';
      dmd.clearScreen(true);
      delay(1000);
      dmd.drawString(0,0,"SLOW", 4, GRAPHICS_NORMAL);  //--> dmd.drawString(x, y, Text, Number of characters in text, GRAPHICS_NORMAL);
      //Code for leds
    }
    break;
  }
  }
}

