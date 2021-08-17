#ifndef FILTER_H
#define FILTER_H

/******************************************************************************
 * median (quickselect) methods
 * Note: All templates must be defined in header file!
 * See https://stackoverflow.com/a/8752879
 *****************************************************************************/

// Get random integer in [lower, upper]
// Credit: https://www.geeksforgeeks.org/generating-random-number-range-c/
int randomInt(int lower, int upper);

// Swap two elements of an array
template <class T>
void swap(T *xp, T *yp) {
    // Remember: * = get value of pointer
    //           & = get pointer of value
    T t = *xp;
    *xp = *yp;
    *yp = t;
}

// Group subarray into two smaller subarrays (< pivotValue, >= pivotValue),
// returning the index of the partition. Uses Lomuto partition scheme, 
// which is simpler but less efficient than Hoare's original partition scheme.
template <class T>
int partition(T arr[], int pivotIndex, int left, int right) {
    // Get value of arr[pivotIndex]
    T pivotValue = arr[pivotIndex];

    // Move pivot to end
    swap<T>(&arr[pivotIndex], &arr[right]);

    // Place all subarray elements < pivotValue on left side of subarray
    int idx = left;
    for (int i = left; i < right; i++) {
        if (arr[i] < pivotValue) {
            swap<T>(&arr[idx], &arr[i]);
            idx++;
        }
    }

    // Move pivot from end to proper place, splitting two new subarrays
    swap<T>(&arr[idx], &arr[right]);

    return idx;
}

// Select k-th greatest element in sorted array within bounds [left, right]
template <class T>
T quickselect(T arr[], int k, int left, int right) {
    // Return element if size(subarray) = 1
    if (left == right) {
        return arr[left];
    }
    
    // Select random pivot (can use median-of-medians for faster time)
    int pivotIndex = randomInt(left, right);

    // Find sorted index of arr[pivotIndex]
    pivotIndex  = partition<T>(arr, pivotIndex, left , right);

    // Further subpartition if needed
    if (k == pivotIndex) {
        // Return element if pivotIndex = k
        return arr[k];
    }
    else if (k < pivotIndex) {
        // Search in left side of array if k > pivotIndex
        return quickselect<T>(arr, k, left, pivotIndex-1);
    }
    else {
        // Search in right side of array if k > pivotIndex
        return quickselect<T>(arr, k, pivotIndex+1, right);
    }
}

/******************************************************************************
 * moving filter methods
 *****************************************************************************/

// Moving filter class
class MovingFilter {

    private:
        long t; // current sample number
        long p; // current pointer in buffer
        double mean; // current buffer mean
        double std; // current buffer std
        double *buffer; // buffer containing filter applied to last n data points
        bool isInitialized; // true if buffer has been properly initialized
        long startIndex = 10; // number of samples after which to start

    public:
        // Filter parameters
        long n; // length of buffer in data points
        long k; // number of samples between buffer updates (rudimentary low-pass filter)
        double thresh; // if method = 'S': number of stds from mean that constitutes signal
                       // if method = 'A': absolute value threshold that constitutes signal
        double alpha; // influence on buffer of most recent data point containing signal
        char method; // signal detection method ('A' = absolute, 'S' = std)
        
        // Constructor
        MovingFilter();
        MovingFilter(long n, long k, double thresh, double alpha, char method);

        // Destructor
        ~MovingFilter() {};

        // Methods
        void createFilter(long n, long k, double thresh, double alpha, char method);
        int applyFilter(double y);
        void reset(void);
        double bufferMean() const { return mean; };
};

// Necessary to link implementation of template classes and methods. 
// See here: https://www.codeproject.com/Articles/48575/How-to-define-a-template-class-in-a-h-file-and-imp
//#include "filter.cpp"

#endif