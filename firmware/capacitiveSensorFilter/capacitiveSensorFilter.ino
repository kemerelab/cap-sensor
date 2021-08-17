#include <CapacitiveSensor.h>
#include <filter.h>

// Settings
const int nSensors = 1; // number of lick sensors
long sensitivity = 30; // sensor sensitivity (increases acquisition time)
bool hasAnalogPin = true; // does setup use analog input pin to determine threshold?
char method = 'A'; // 'A' = use absolute threshold, 'S' = use std threshold
double thresh; // threshold for detecting lick (stds from mean or raw value)
double thresh_low = 500.0; 
double thresh_high = 10000.0;
double alpha = 0.000; // contribution of signal to filter buffer stats
double tFilter = 20.0; // duration of filter buffer (s) (determines responsiveness)
double sampleRate = 200.0; // sample frequency (Hz) (determines resolution)
long k = 10; // downsample factor; update buffer every k samples

// Sensor I/O (constructor format: (pin_HIGH, pin_LOW))
CapacitiveSensor cs[] = {CapacitiveSensor(0, 1)};
long val; // sensor value
int pinsOut[nSensors] = {3}; // output pins
int ledsOut[nSensors] = {13}; // LEDs representing sensor states
int output[nSensors]; // current output states
int analogPin = A1; // threshold input

// Sensor filter
MovingFilter mf[nSensors]; // filters
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
        pinMode(ledsOut[i], OUTPUT);
        digitalWrite(pinsOut[i], LOW);
        digitalWrite(ledsOut[i], LOW);
        output[i] = LOW;
    }

    // Blink LED on/off five times (debugging)
    // for (int j = 0; j < 5; j++) {
    //     for (int i = 0; i < nSensors; i++) {
    //         digitalWrite(ledsOut[i], HIGH);
    //     }
    //     delay(500);
    //     for (int i = 0; i < nSensors; i++) {
    //         digitalWrite(ledsOut[i], LOW);
    //     }
    //     delay(500);
    // }

    // Set analog input for threshold
    if (hasAnalogPin) {
        pinMode(analogPin, INPUT);
        thresh = getThreshold(analogRead(analogPin));
    }
    
    // Begin serial transmission
    Serial.begin(115200);

    // Raise error if 1) sample rate > 0.002 / (nSensors * sensitivity/30), or 
    // 2) buffer size > 1500
    double sampleRateMax = (1.0 / (2.0e-3 * (double) nSensors * ((double) sensitivity / 30.0)));
    if (sampleRate > sampleRateMax) {
        Serial.print("sampleRate of "); Serial.print(sampleRate); 
        Serial.print(" exceeds maximum of "); Serial.println(sampleRateMax);
        Serial.print("Changing sampleRate to "); Serial.println(sampleRateMax);
        sampleRate = sampleRateMax;
    }
    long n = (long) (tFilter * sampleRate / (double) k);
    int nMax = (long) (1500.0 / ((double) nSensors));
    if (n > nMax) {
        Serial.print("buffer size of "); Serial.print(n); 
        Serial.print(" exceeds maximum of "); Serial.println(nMax);
        Serial.print("Changing buffer size to "); Serial.println(nMax);
        n = nMax;
    }

    // Create filters
    for (int i = 0; i < nSensors; i++) {
        mf[i].createFilter(n, k, thresh, alpha, method);
    }

    // Get start time and minimum sample period
    tStart = micros(); // time in ms
    samplePeriod = (unsigned long) ((1.0/sampleRate)*1e6); // period in us 

    // Print parameters before beginning loop
    Serial.println("Starting sensor with parameters:");
    Serial.print("buffer size: "); Serial.println(n);
    Serial.print("buffer duration: "); Serial.println((double) n * samplePeriod * 1.0e-6 * (double) k);
    Serial.print("buffer update rate: "); Serial.println(sampleRate / (double) k);
    Serial.print("sample rate: "); Serial.println(sampleRate);
    Serial.print("alpha: "); Serial.println(alpha, 5);
    if (hasAnalogPin) {
        Serial.print("signal threshold range: "); Serial.print(thresh_low, 2); Serial.print(" - "); Serial.println(thresh_high, 2);
    }
    else {
        Serial.print("signal threshold: "); Serial.println(thresh, 2);
    }
    Serial.println();
}

void loop() {
    // Get time
    tCurrent = micros();
    
    // Check command
    // TODO: freezes with Trinket M0 for some reason...
    // if (Serial.available() > 0) {
    //     processCommand();
    // }

    // Write sensor value with sample frequency or if clock restarted
    if ((tCurrent - tStart > samplePeriod) || (tCurrent - tStart < 0))
    {
        //Serial.print(tCurrent);
        Serial.print(tCurrent);

        for (int i = 0; i < nSensors; i++)
        {
            // Get current sensor value
            // Note: sensor value acquisition limits transmission rate
            val = cs[i].capacitiveSensorRaw(sensitivity); // use raw value to avoid heuristic conflicting with filter
      
            // Print raw values
            Serial.print(" ");
            Serial.print(val);

            // Get current threshold and update filter
            if (hasAnalogPin) {
                thresh = getThreshold(analogRead(analogPin));
                mf[i].thresh = thresh;
            }

            // Print threshold value
            Serial.print(" ");
            Serial.print(thresh);

            // Print buffer mean (baseline)
            Serial.print(" ");
            Serial.print(mf[i].bufferMean());

            // Get filtered values
            x = mf[i].applyFilter((double) val);

            // NOTE: Do not call reset() after a "negative" signal! This will
            // cause many timeouts within a lick bout as the negative component
            // of the fluctuation may exceed threshold and thus call the reset()
            // function often, which triggers a delay as the buffer refills.
            //if (x == -1) {
            //    mf[i].reset();
            //}
      
            // Print filtered values
            Serial.print(" ");
            Serial.print(x);
            
            // Write digital I/O if threshold crossed
            if ((x == 1) && (output[i] == LOW))
            {
                digitalWrite(pinsOut[i], HIGH);
                digitalWrite(ledsOut[i], HIGH);
                output[i] = HIGH;
            }
            else if ((x < 1) && (output[i] == HIGH))
            {
                digitalWrite(pinsOut[i], LOW);
                digitalWrite(ledsOut[i], LOW);
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
      Serial.print("Setting alpha to "); Serial.println(alpha, 5);
      for (int i = 0; i < nSensors; i++) {
        mf[i].alpha = alpha;
      }
      delay(5000);
    }
    else if (strcmp(name, "thresh") == 0) {
      thresh = atof(val);
      Serial.print("Setting threshold to "); Serial.println(thresh, 2);
      for (int i = 0; i < nSensors; i++) {
        mf[i].thresh = thresh;
      }
      delay(5000);
    }
    else {
      Serial.print("Unknown command: ");
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

float getThreshold(int val) {
    // analogRead() range 0-1023 (10-bit ADC)
    //return (long) ((float(val)/1023)*(thresh_high - thresh_low) + thresh_low); // directly proportional
    return (long) ((1.0 - (float(val)/1023))*(thresh_high - thresh_low) + thresh_low); // inversely proportional
}