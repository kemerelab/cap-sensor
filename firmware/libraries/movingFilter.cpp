/******************************************************************************
 * This script tests the algorithm to extract a noisy signal by reading
 * from a data file and logging the signal as output.
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "filter.h"

int main(int argc, char *argv[]) {
    // Create new filter
    int bufferSize = atoi(argv[1]);
    double thresh = atof(argv[2]);
    double alpha = atof(argv[3]);
    int k = atof(argv[4]);
    char method = *argv[5];
    //char method = 'S';
    MovingFilter filter(bufferSize, k, thresh, alpha, method);
    
    // Read file
    char *inFilename = argv[6];
    std::ifstream inFile(inFilename);
    printf("Reading from file %s\n", inFilename);

    // Output to file
    char *outFilename = argv[7];
    std::ofstream outFile(outFilename);
    printf("Logging to file %s\n", outFilename);

    // Stream input to filter
    if (inFile.is_open()) {
        //int x; // signal placeholder
        double y; // raw data placeholder

        while(inFile >> y) {
            // Output to file
            outFile << filter.applyFilter(y) << "\n";
        }
    }

    return(0);
}        
        