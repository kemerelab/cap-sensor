/******************************************************************************
 * This script tests the algorithm to find the median in a given array.
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "filter.h"

int main(int argc, char *argv[]) {
    /*
    // NOTE: In C, arrays are passed as pointers, so size information is lost.
    // If another function needs array size information, it must be passed as
    // a separate parameter.
    long arr[] = {7, 5, 2, 3, 4, 1, 9, 6, 3, 5, 6};
    int k;
    char buf[8];
    
    // Get array size
    int N = sizeof(arr) / sizeof(arr[0]);

    // Get k from stdin
    printf ("Enter your number: ");
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        k = atoi(buf);
    }
    else {
        k = (int) sizeof(arr) / sizeof(arr[0]) / 2;
    }
    */

    // Get command line arguments
    int N = atoi(argv[1]); // length of array
    int k = atoi(argv[2]); // k-th median
    int M = atoi(argv[3]); // number of iterations

    // Create random array
    long arr[N];
    srand(time(NULL));
    for (int n = 0; n < N; n++) {
        arr[n] = rand();
    }

    // Time loops
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int m = 0; m < M; m++) {
        // Perform quickselect algorithm
        long median = quickselect(arr, k, 0, N-1);

        // Print result
        //printf("%d th element of array is %ld.\n", k, median);
    }
    gettimeofday(&end, NULL);
    double tTotal = ((double) (end.tv_sec - start.tv_sec)) + 10e-6*((double) (end.tv_usec - start.tv_usec));
    double tLoop = tTotal / (double) M;
    printf("Total time: %lf s (%lf s per loop)\n", tTotal, tLoop);

    // Exit
    return(0);
}