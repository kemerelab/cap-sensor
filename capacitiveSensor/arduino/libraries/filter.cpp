#include <stdlib.h>
#include <stdint.h>
//#include <cmath> // g++
#include<math.h> // arduino
#include "filter.h"

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
MovingFilter::MovingFilter(long n, double thresh, double alpha) {
    MovingFilter::createFilter(n, thresh, alpha);
}

// Post hoc construction
void MovingFilter::createFilter(long n, double thresh, double alpha) {
    // Set parameters
    this->n = n;
    this->thresh = thresh;
    this->alpha = alpha;

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

    // Check for signal (if buffer filled)
    if ((t > n) && (abs(y - mean) > (thresh*std))) {
        // Update signal array
        if ((y - mean) > 0.0) { // positve signal
            x = 1;
        }
        else { // negative signal
            x = -1;
        }

        // Update buffer
        long prev = ((p - 1) + n) % n; // wraps to end if needed
        buffer[p] = (alpha*y) + ((1.0 - alpha)*buffer[prev]);
    }
    else {
        // Update signal and buffer
        x = 0;
        buffer[p] = y;
    }

    // Update buffer stats
    if (t % 1000 == 0) {
        // Get ground truth stats occasionally to correct drift
        mean = 0;
        for (int i = 0; i < n; i++) {
            mean += buffer[i];
        }
        mean = mean / (double) n;
        std = 0;
        for (int i = 0; i < n; i++){
            std += pow((buffer[i] - mean), 2);
        }
        std = sqrt(std / (double) n);
    }
    else {
        // Use single point updates for faster processing
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

    // Increment indices
    p = (p + 1) % n;
    t++;

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