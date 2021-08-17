#include <stdlib.h>
#include <stdint.h>
//#include <stdexcept>
#include <string>
//#include <cmath> // g++
#include<math.h> // arduino
#include "filter.h"
#include <Arduino.h>

/******************************************************************************
 * median (quickselect) methods
 *****************************************************************************/

// Get random integer in [lower, upper]
// Credit: https://www.geeksforgeeks.org/generating-random-number-range-c/
int randomInt(int lower, int upper) {
    return (rand() % (upper - lower + 1)) + lower;
}

/******************************************************************************
 * moving filter methods
 *****************************************************************************/

// Default constructor
MovingFilter::MovingFilter(void) {
    isInitialized = false;
}

// Parameterized constructor
MovingFilter::MovingFilter(long n, long k, double thresh, double alpha, char method) {
    MovingFilter::createFilter(n, k, thresh, alpha, method);
}

// Post hoc construction
void MovingFilter::createFilter(long n, long k, double thresh, double alpha, char method) {
    // Check method
    if ((method != 'A') && (method != 'S')) {
        // Print error message
        //std::string msg("invalid method ");
        //msg.push_back(method);
        //throw std::invalid_argument(msg);
        Serial.print("Unknown method '"); Serial.print(method); Serial.print("'. Defaulting to 'A'.");
        method = 'A';
    }

    // Set parameters
    this->n = n;
    this->k = k;
    this->thresh = thresh;
    this->alpha = alpha;
    this->method = method;

    // Set variables
    t = 0;
    p = 0;
    mean = 0.0;
    std = 0.0;

    // Allocate buffer
    buffer = new double[n];

    // Ready to go
    isInitialized = true;
}

// Apply filter to next data sample
int MovingFilter::applyFilter(double y) { 
    if (!isInitialized) {
        return INT16_MAX;
    }

    // Signal value
    int x;
    
    // Cache old value
    double bufferOld = buffer[p];
    double meanOld = mean;

    // Update buffer every k values
    bool updateBuffer = (t % k == 0);
    long m = t / k; // current sample number

    // Check for signal (if received adequate number of samples to start)
    if ( (m >= startIndex) &&
         (((abs(y - mean) > (thresh*std)) && (method == 'S')) ||
          ((abs(y - mean) > thresh) && (method == 'A'))) ) {
        // Update signal array
        if ((y - mean) > 0.0) { // positve signal
            x = 1;
        }
        else { // negative signal
            x = -1;
        }

        // Update buffer
        if (updateBuffer) {
            long prev = ((p - 1) + n) % n; // wraps to end if needed
            buffer[p] = (alpha*y) + ((1.0 - alpha)*buffer[prev]);
        }
    }
    else {
        // Update signal and buffer
        x = 0;
        if (updateBuffer) {
            buffer[p] = y;
        }
    }

    // Update buffer stats
    if (updateBuffer) {
        if ((m == n) || (m == startIndex)) {
            // Get ground truth stats upon initialization
            mean = 0.0;
            for (int i = 0; i < m; i++) {
                mean += buffer[i];
            }
            mean = mean / (double) m;
            std = 0.0;
            for (int i = 0; i < m; i++){
                std += pow((buffer[i] - mean), 2);
            }
            std = sqrt(std / (double) m);
        }
        else if ((m > n) && (m % 1000 == 0)) {
            // Get ground truth stats 1) when buffer first filled or 
            // 2) occasionally to correct drift
            mean = 0.0;
            for (int i = 0; i < n; i++) {
                mean += buffer[i];
            }
            mean = mean / (double) n;
            std = 0.0;
            for (int i = 0; i < n; i++){
                std += pow((buffer[i] - mean), 2);
            }
            std = sqrt(std / (double) n);
        }
        else if (m > n) {
            // Use single point updates for faster processing (replacing item in buffer)
            double dMean = (1.0/(double) n)*(buffer[p] - bufferOld);
            mean += dMean;
            std = (pow(std, 2) - (1.0/(double) n)*pow(bufferOld - meanOld, 2) 
                + (1.0/(double) n)*pow(buffer[p] - mean, 2) 
                + (2.0*dMean/(double) n)*(bufferOld - meanOld) 
                + (((double) n - 1)/((double) n))*pow(dMean, 2));
            if (std < 0.0) {
                // Avoids sqrt(-0.0) = NaN in case of rounding error
                std = 0.0;
            }
            else {
                std = sqrt(std);
            }      
        }
        else if (m > startIndex) {
            // Use single point updates for faster processing (adding new item to buffer)
            mean = ((double) (m - 1))*meanOld + buffer[p];
            mean /= (double) m;
            std = ((double) (m - 1))*pow(std, 2) + (buffer[p] - mean)*(buffer[p] - meanOld);
            std /= (double) m;
            if (std < 0.0) {
                // Avoids sqrt(-0.0) = NaN in case of rounding error
                std = 0.0;
            }
            else {
                std = sqrt(std);
            }
        }
    }

    // Increment indices
    t++;
    if (updateBuffer) {
        p = (p + 1) % n;
    }

    // Return signal
    return x;
}


// This method may be helpful to reset the filter if a negative signal
// is returned when it should always be positive, or vice versa.
void MovingFilter::reset(void) {
    // Reset buffer
    for (int i = 0; i < n; i++) {
        buffer[i] = 0.0;
    }
    p = 0;

    // Reset buffer stats
    mean = 0.0;
    std = 0.0;

    // Reset global counter
    t = 0;
}