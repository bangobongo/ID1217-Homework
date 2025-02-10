#include <stdio.h>
#include <stdlib.h>

// Function to compare two double values (for qsort)
int compareDoubles(const void *a, const void *b) {
    double diff = (*(double *)a - *(double *)b);
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

// Function to calculate the median of an array of doubles
double calculateMedian(double *array, int size) {
    // Sort the array
    qsort(array, size, sizeof(double), compareDoubles);

    // Calculate the median
    if (size % 2 == 1) {
        // If the size is odd, return the middle element
        return array[size / 2];
    } else {
        // If the size is even, return the average of the two middle elements
        return (array[size / 2 - 1] + array[size / 2]) / 2.0;
    }
}

int main() {
    // Define matrix sizes and thread sizes
    int matrixSizes[] = {10000, 5000, 1000};
    int threadSizes[] = {8, 4, 2, 1};
    int numMatrixSizes = sizeof(matrixSizes) / sizeof(matrixSizes[0]);
    int numThreadSizes = sizeof(threadSizes) / sizeof(threadSizes[0]);
    int numRuns = 25; // Number of runs per combination

    // Loop over all combinations of matrix sizes and thread sizes
    for (int m = 0; m < numMatrixSizes; m++) {
        int currentMatrix = matrixSizes[m];

        for (int t = 0; t < numThreadSizes; t++) {
            int currentThreads = threadSizes[t];

            // Array to store the output of each run
            double results[numRuns];

            printf("Testing matrix size %d with thread size %d:\n", currentMatrix, currentThreads);

            for (int i = 0; i < numRuns; i++) {
                // Construct the command string dynamically
                char command[100];
                sprintf(command, "matrixSum.exe %d %d", currentMatrix, currentThreads);

                // Open a pipe to the command and read its output
                FILE *pipe = popen(command, "r");
                if (!pipe) {
                    printf("Failed to run command: %s\n", command);
                    continue;
                }

                // Read the output of the command
                char buffer[128];
                if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                    // Convert the output to a double
                    results[i] = atof(buffer);
                    printf("  Run %d: Output = %.6f\n", i + 1, results[i]);
                } else {
                    printf("Failed to read output from command: %s\n", command);
                    results[i] = 0.0; // Default value in case of failure
                }

                // Close the pipe
                pclose(pipe);
            }

            // Calculate the median of the results
            double median = calculateMedian(results, numRuns);
            printf("\n \n  Median of all runs: \n %.6f\n\n", median);
        }
    }

    return 0;
}