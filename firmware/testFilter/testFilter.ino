#include <CapacitiveSensor.h>
#include <filter.h>

// To test the filter code, change `#include <math.h>` to `#include <cmath>` 
// in `filter.cpp`, and compile `medianFilter.cpp` and/or `movingFilter.cpp`.

// Settings
const int nSensors = 2; // number of lick sensors
long sensitivity = 120; // sensor sensitivity
double thresh = 4.0; // threshold for detecting lick (stds from mean)
double alpha = 0.0; // contribution of signal to filter buffer stats
double tFilter = 0.25; // duration of filter buffer (s) (determines responsiveness)
double sampleRate = 500.0; // sample frequency (Hz) (determines resolution)

// Raise error if 1) buffer size > 1500 or 2) sample rate > 500 / nSensors

// Sensor I/O (constructor format: (pin_HIGH, pin_LOW))
CapacitiveSensor cs[] = {
  CapacitiveSensor(13, 11),
  CapacitiveSensor(4, 2)
};
long val; // sensor value
int pinsOut[nSensors] = {22, 23}; // output pins
int output[nSensors]; // current output states

// Sensor filter
long n = (long) (tFilter * sampleRate);
MovingFilter mf[] = {
  MovingFilter(n, thresh, alpha),
  MovingFilter(n, thresh, alpha)
};
int x; // filter signal

// Timing variables
unsigned long tStart;
unsigned long tCurrent; 
unsigned long samplePeriod; 

// Command setup
const unsigned int MAX_LENGTH = 100;
char cmd[MAX_LENGTH];
unsigned int cmdPointer = 0;

void setup() {
  // Set pins to low voltage
  for (int i = 0; i < nSensors; i++)
  {
    pinMode(pinsOut[i], OUTPUT);
    digitalWrite(pinsOut[i], LOW);
    output[i] = LOW;
  }

  // Begin serial transmission
  Serial.begin(115200);
  Serial.println(n);
  
  // Get start time and minimum sample period
  tStart = micros(); // time in ms
  samplePeriod = (unsigned long) ((1.0/sampleRate)*1e6); // period in us 
  Serial.println(samplePeriod);
}

void loop() {
  // Get time
  tCurrent = micros();
  
  // Check command
  if (Serial.available() > 0) {
    processCommand();
  }

  // Write sensor value with sample frequency or if clock restarted
  if ((tCurrent - tStart > samplePeriod) || (tCurrent - tStart < 0))
  {
    //Serial.print(tCurrent);
    Serial.print(tCurrent - tStart);

    for (int i = 0; i < nSensors; i++)
    {
      // Get current sensor value
      // Note: sensor value acquisition limits transmission rate
      val = cs[i].capacitiveSensor(sensitivity);
      
      // Print raw values
      Serial.print(" ");
      Serial.print(val);

      // Get filtered values
      x = mf[i].applyFilter((double) val);
      if (x == -1) {
        mf[i].reset();
      }
      
      // Print filtered values
      Serial.print(" ");
      Serial.print(x);
      // Further debugging
      // NOTE: These class variables must be made public in
      // the header file to be printed here.
      /*
      Serial.print(" ");
      Serial.print(mf[i].mean);
      Serial.print(" ");
      Serial.print(mf[i].std);
      Serial.print(" ");
      Serial.print(mf[i].t);
      Serial.println();
      for (int j = 0; j < 10; j++) {
        Serial.print(mf[i].buffer[j]);
        Serial.print(" ");
      }
      Serial.print("\n");
      */
      
      // Write digital I/O if threshold crossed
      if ((x == 1) && (output[i] == LOW))
      {
        digitalWrite(pinsOut[i], HIGH);
        output[i] = HIGH;
      }
      else if ((x < 1) && (output[i] == HIGH))
      {
        digitalWrite(pinsOut[i], LOW);
        output[i] = LOW;
      }
    }
    Serial.print("\n");
    tStart = tCurrent;
  }
  
}

void processCommand(void) {
  // Add char to buffer
  char c = Serial.read();
  
  // Process command if newline; otherwise, add to char array
  if (c == '\n') {
    // Find whitespace to separate command name and value
    int j;
    for (int i = 0; i < cmdPointer; i++) {
      if (cmd[i] == ' ') {
        j = i;
        break;
      }
    }

    // Parse command name and value
    char name[j+1];
    memcpy(name, &cmd, j*sizeof(*cmd));
    name[j] = '\0';
    char val[cmdPointer-j];
    memcpy(val, &cmd[j+1], (cmdPointer-j-1)*sizeof(*cmd));
    val[cmdPointer-j-1] = '\0';

    // Set new value
    if (strcmp(name, "alpha") == 0) {
      alpha = atof(val);
      for (int i = 0; i < nSensors; i++) {
        mf[i].alpha = alpha;
      }
    }
    else if (strcmp(name, "thresh") == 0) {
      thresh = atof(val);
      for (int i = 0; i < nSensors; i++) {
        mf[i].thresh = thresh;
      }
    }
    else {
      Serial.println("Unknown command:");
      Serial.println(cmd);
    }

    // Update command variables
    cmdPointer = 0;
  }
  else {
    // add to cmd
    if (cmdPointer < MAX_LENGTH) {
      cmd[cmdPointer] = c;
      cmdPointer++;
    }
    else {
      Serial.println("Maximum command length exceeded.");
      cmdPointer = 0;
    }
  }
}