/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
	 gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
	 ./matrixSum-openmp size numWorkers

*/

/*
Download, compile, and run a program matrixSum-openmp.c Download matrixSum-openmp.c.
The program computes a sum of matrix elements in parallel using OpenMP.
Develop and evaluate the following modified version of the program.
Extend the program so that in addition to the sum, it finds and prints a value and a position (indexes)
of the maximum element of the matrix and a value and a position of the minimum element of the matrix.
To check your solution, initialize elements of the matrix to random values. Use OpenMP constructs.

Run the program on different numbers of processors and report the speedup
(sequential execution time divided by parallel execution time) for different numbers of processors
(up to at least 4) and different sizes of matrices (at least 3 different sizes).
Run each program several (at least 5) times and use the median value for execution time.
Try to provide reasonable explanations for your results. Measure only the parallel part of your program.
Specify the number of processors used by specifying a different number of threads
(set the OMP_NUM_THREADS environment variable or use a call to omp_set_num_threads(),
see the OpenMP specification). To measure the execution time, use the omp_get_wtime function
(see omp_get_wtimeLinks to an external site.).
*/

/*
	--------------TEST RESULTS----------------
	+-------------+---------+----------------+
	| Matrix Size | Threads | Time (Median)  |
	+-------------+---------+----------------+
	| 10000       | 8       | 0.072          |
	| 10000       | 4       | 0.064          |
	| 10000       | 2       | 0.119          |
	| 10000       | 1       | 0.236          |
	|-------------|---------|----------------|
	| 5000        | 8       | 0.018          |
	| 5000        | 4       | 0.017          |
	| 5000        | 2       | 0.031          |
	| 5000        | 1       | 0.062          |
	|-------------|---------|----------------|
	| 1000        | 8       | 0.002          |
	| 1000        | 4       | 0.002          |
	| 1000        | 2       | 0.002          |
	| 1000        | 1       | 0.003          |
	+-------------+---------+----------------+

	25 runs were performed per matrix size to thread combination. The results show quite significant speedup
	from 1 to 8 threads, approximately a 3.3x speedup on matrix size 10 000 and a 3.4x speedup on 5 000. 
	The results for matrix size 1 are too close to the measuring resolution to draw any meaningful conclusions. 
	Of note is that 4 threads outperform 8 threads slightly across every matrix size. The "O0" compile flag was used,
	so it cannot be due to compiler optimizations. However, it might be due to thread initialization or
	synchronization overhead. This overhead might become less significant at larger matrix sizes and then 8 threads 
	would be faster, but this was not tested.

*/

#include <omp.h>

double start_time, end_time;

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#define MAXSIZE 10000 /* maximum matrix size */
#define MAXWORKERS 8  /* maximum number of workers */
#define MODULO 99

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];

int max = 0;
int maxX;
int maxY;

int min = INT_MAX;
int minX;
int minY;

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	int i, j, total = 0;

	/* read command line args if any */
	size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
	numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
	if (size > MAXSIZE)
		size = MAXSIZE;
	if (numWorkers > MAXWORKERS)
		numWorkers = MAXWORKERS;

	omp_set_num_threads(numWorkers);

	srand(time(NULL));

	/* initialize the matrix */
	for (i = 0; i < size; i++)
	{
		//  printf("[ ");
		for (j = 0; j < size; j++)
		{
			matrix[i][j] = rand() % MODULO;
			//	  printf(" %d", matrix[i][j]);
		}
		//	  printf(" ]\n");
	}

start_time = omp_get_wtime();

#pragma omp parallel for reduction(+ : total) private(j)
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
		{
			total += matrix[i][j];
		}


#pragma omp parallel
	{
		int localMin = INT_MAX; // e.g., INT_MAX
		int localMax = 0;		// e.g., INT_MIN
		int localMinX, localMinY, localMaxX, localMaxY;

#pragma omp for
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				if (matrix[i][j] < localMin)
				{
					localMin = matrix[i][j];
					localMinX = i;
					localMinY = j;
				}
				if (matrix[i][j] > localMax)
				{
					localMax = matrix[i][j];
					localMaxX = i;
					localMaxY = j;
				}
			}
		}

		// Instructions: "Measure only the parallel part of your program."
		end_time = omp_get_wtime();

// After the for-loop, merge local results into global variables:
#pragma omp critical
		{
			if (localMin < min)
			{
				min = localMin;
				minX = localMinX;
				minY = localMinY;
			}
			if (localMax > max)
			{
				max = localMax;
				maxX = localMaxX;
				maxY = localMaxY;
			}
		}
	}

	for (i = 0; i < MAXSIZE; i++)
	{
		for (j = 0; j < MAXSIZE; j++)
		{
			// printf("value: %d at pos: [%d][%d]  \n", matrix[i][j], i, j);
		}
	}

	// printf("the total is %d\n", total);
	// printf("the max is %d, located in position [%d][%d]\n", max, maxX, maxY);
	// printf("the min is %d, located in position [%d][%d]\n", min, minX, minY);
	// printf("it took %g seconds\n", end_time - start_time);
	printf("%g \n", end_time - start_time);
}
