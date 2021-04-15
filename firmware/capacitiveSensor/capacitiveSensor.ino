#include <CapacitiveSensor.h>

// Settings
int nSensors = 1; // number of lick sensors
long th = 100; // threshold for detecting lick
float sampleRate = 30000.0; // rate to print to serial

// Sensor I/O (constructor format: (pin_HIGH, pin_LOW))
CapacitiveSensor cs[] = {
  CapacitiveSensor(1, 0)
};
long vals[1]; // sensor values
int pinsOut[1] = {2}; // output pins
int output[1] = {LOW}; // current output states

// Timing variables
unsigned long tStart;
unsigned long tCurrent; 
unsigned long samplePeriod; 

void setup() {
  // Set pins to low voltage
  for (int i = 0; i < nSensors; i++)
  {
    pinMode(pinsOut[i], OUTPUT);
    digitalWrite(pinsOut[i], LOW);
  }

  // Begin serial transmission
  Serial.begin(115200);
  
  // Get start time and minimum sample period
  tStart = micros(); // time in ms
  samplePeriod = (unsigned long) ((1.0/sampleRate)*1e6); // period in us
}

void loop() {
  // Get current sensor value and time
  // Note: sensor value acquisition limits transmission rate
  for (int i = 0; i < nSensors; i++)
  {
    vals[i] = cs[i].capacitiveSensor(30);
  }
  tCurrent = micros();

  // Write sensor value with sample frequency or if clock restarted
  if ((tCurrent - tStart > samplePeriod) || (tCurrent - tStart < 0))
  {
    Serial.print(tCurrent);
    for (int i = 0; i < nSensors; i++)
    {
      Serial.print(" ");
      Serial.print(vals[i]);
    }
    Serial.print("\n");
    tStart = tCurrent;
  }
  
  // Write digital I/O if threshold crossed
  for (int i = 0; i < nSensors; i++)
  {
    if ((vals[i] > th) && (output[i] == LOW))
    {
      digitalWrite(pinsOut[i], HIGH);
      output[i] = HIGH;
    }
    else if ((vals[i] < th) && (output[i] == HIGH))
    {
      digitalWrite(pinsOut[i], LOW);
      output[i] = LOW;
    }
  }
}
