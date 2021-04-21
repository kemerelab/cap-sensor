#include <CapacitiveSensor.h>

// Settings
int nSensors = 1; // number of lick sensors
long th; // threshold for detecting lick
float th_low = 10.0;
float th_high = 10000.0;
float sampleRate = 30000.0; // rate to print to serial

// Sensor I/O (constructor format: (pin_HIGH, pin_LOW))
CapacitiveSensor cs[] = {
  CapacitiveSensor(0, 1)
};
long vals[1]; // sensor values
int pinsOut[1] = {3}; // output pins
int ledsOut[1] = {13}; // LEDs representing sensor states
int output[1] = {LOW}; // current output states
int analogPin = A1; // threshold input

// Timing variables
unsigned long tStart;
unsigned long tCurrent; 
unsigned long samplePeriod; 

void setup() {
  // Set pins to low voltage
  for (int i = 0; i < nSensors; i++)
  {
    pinMode(pinsOut[i], OUTPUT);
    pinMode(ledsOut[i], OUTPUT);
    digitalWrite(pinsOut[i], LOW);
    digitalWrite(ledsOut[i], LOW);
  }

  // Set analog input for threshold
  pinMode(analogPin, INPUT);

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

  // Get current threshold
  th = getThreshold(analogRead(analogPin));

  // Write sensor value with sample frequency or if clock restarted
  if ((tCurrent - tStart > samplePeriod) || (tCurrent - tStart < 0))
  {
    Serial.print(tCurrent);
    for (int i = 0; i < nSensors; i++)
    {
      Serial.print(" ");
      Serial.print(vals[i]);
    }
    Serial.print(" ");
    Serial.print(th);
    Serial.print("\n");
    tStart = tCurrent;
  }
  
  // Write digital I/O if threshold crossed
  for (int i = 0; i < nSensors; i++)
  {
    if ((vals[i] > th) && (output[i] == LOW))
    {
      digitalWrite(pinsOut[i], HIGH);
      digitalWrite(ledsOut[i], HIGH);
      output[i] = HIGH;
    }
    else if ((vals[i] < th) && (output[i] == HIGH))
    {
      digitalWrite(pinsOut[i], LOW);
      digitalWrite(ledsOut[i], LOW);
      output[i] = LOW;
    }
  }
}

float getThreshold(int val) {
    // analogRead() range 0-1023 (10-bit ADC)
    //return (long) ((float(val)/1023)*(th_high - th_low) + th_low); // directly proportional
    return (long) ((1.0 - (float(val)/1023))*(th_high - th_low) + th_low); // inversely proportional
}
