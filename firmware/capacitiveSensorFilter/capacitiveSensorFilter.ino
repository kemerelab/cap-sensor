#include <CapacitiveSensor.h>
#include <filter.h>

// Settings
const int nSensors = 2; // number of lick sensors
long sensitivity = 30; // sensor sensitivity (increases acquisition time)
double thresh = 5.0; // threshold for detecting lick (stds from mean)
double alpha = 0.007; // contribution of signal to filter buffer stats
double tFilter = 0.500; // duration of filter buffer (s) (determines responsiveness)
double sampleRate = 200.0; // sample frequency (Hz) (determines resolution)

// Sensor I/O (constructor format: (pin_HIGH, pin_LOW))
CapacitiveSensor cs[] = {
    CapacitiveSensor(13, 11),
    CapacitiveSensor(4, 2)
};
long val; // sensor value
int pinsOut[nSensors] = {22, 23}; // output pins
int output[nSensors]; // current output states

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
        digitalWrite(pinsOut[i], LOW);
        output[i] = LOW;
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
    long n = (long) (tFilter * sampleRate);
    int nMax = (long) (1500.0 / ((double) nSensors));
    if (n > nMax) {
        Serial.print("buffer size of "); Serial.print(n); 
        Serial.print(" exceeds maximum of "); Serial.println(nMax);
        Serial.print("Changing buffer size to "); Serial.println(nMax);
        n = nMax;
    }

    // Create filters
    for (int i = 0; i < nSensors; i++) {
        mf[i].createFilter(n, thresh, alpha);
    }

    // Get start time and minimum sample period
    tStart = micros(); // time in ms
    samplePeriod = (unsigned long) ((1.0/sampleRate)*1e6); // period in us 

    // Print parameters before beginning loop
    Serial.println("Starting sensor with parameters:");
    Serial.print("buffer size: "); Serial.println(n);
    Serial.print("buffer duration: "); Serial.println((double) n * samplePeriod * 1.0e-6);
    Serial.print("sample rate: "); Serial.println(sampleRate);
    Serial.print("alpha: "); Serial.println(alpha, 5);
    Serial.print("signal threshold (std): "); Serial.println(thresh, 2);
    Serial.println();

    delay(5000);
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
        Serial.print(tCurrent);

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