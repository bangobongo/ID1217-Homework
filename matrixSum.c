/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
			 the total sum from partial sums computed by Workers
			 and prints the total sum to the standard output

   usage under Linux:
	 gcc matrixSum.c -lpthread 
	 a.out size numWorkers

*/

/*
The purpose of this problem is to introduce you to the Pthreads library. Download, compile, and run a program matrixSum.c Download matrixSum.c.
The program computes a sum of matrix elements in parallel using Pthreads. Develop the following three modified versions of the program.

(a) Extend the program so that in addition to the sum, it finds and prints a value and a position (indexes) of
the maximum element of the matrix and a value and a position of the minimum element of the matrix.
To check your solution, initialize matrix elements to random values (rather than all ones like in the above example).

(b) Change the program developed in (a) so that the main thread prints the final results.
Do not call the Barrier function, and do not use arrays for partial results, such as sums in the above example.

(c) Change the program developed in (b) so that it uses a "bag of tasks" that is represented as a row counter, which is initialized to 0.
A worker gets a task (i.e., the number of the row to process) out of the bag by reading a value of the counter and incrementing the
counter as described for the matrix multiplication problem in Slides 27-29 about the "bag of tasks" in Lecture 5.

*/
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000 /* maximum matrix size 10000*/
#define MAXWORKERS 10 /* maximum number of workers */

pthread_mutex_t barrier; /* mutex lock for the barrier */
pthread_cond_t go;		 /* condition variable for leaving */
int numWorkers;			 /* number of workers */
int numArrived = 0;		 /* number who have arrived */

// my work - global max/min and pos
int globalMax = 0;
int gMaxPosX;
int gMaxPosY;

int globalMin = RAND_MAX;
int gMinPosX;
int gMinPosY;

// my work - mutexes
pthread_mutex_t mutex_localMax = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_localMin = PTHREAD_MUTEX_INITIALIZER;

/* a reusable counter barrier */
void Barrier()
{
	pthread_mutex_lock(&barrier);
	numArrived++;
	if (numArrived == numWorkers)
	{
		numArrived = 0;
		pthread_cond_broadcast(&go);
	}
	else
		pthread_cond_wait(&go, &barrier);
	pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer()
{
	static bool initialized = false;
	static struct timeval start;
	struct timeval end;
	if (!initialized)
	{
		gettimeofday(&start, NULL);
		initialized = true;
	}
	gettimeofday(&end, NULL);
	return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time;  /* start and end times */
int size, stripSize;		  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS];		  /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

// my work - local max/mins and pos
int localMax[MAXWORKERS][3];
int localMin[MAXWORKERS][3];

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	int i, j;
	long l; /* use long in case of a 64-bit system */
	pthread_attr_t attr;
	pthread_t workerid[MAXWORKERS];

	/* set global thread attributes */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	/* initialize mutex and condition variable */
	pthread_mutex_init(&barrier, NULL);
	pthread_cond_init(&go, NULL);

	/* read command line args if any */
	size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
	numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;
	if (size > MAXSIZE)
		size = MAXSIZE;
	if (numWorkers > MAXWORKERS)
		numWorkers = MAXWORKERS;
	stripSize = size / numWorkers;

	// randomize rand() to get new values each time
	srand(time(NULL));

	/* initialize the matrix */
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			matrix[i][j] = rand() % 99; // 10 000 000;
		}
	}

	// testing
	//matrix[6][16] = 150;

	/* print the matrix */
#ifdef DEBUG
	for (i = 0; i < size; i++)
	{
		printf("[ ");
		for (j = 0; j < size; j++)
		{
			printf(" %d", matrix[i][j]);
		}
		printf(" ]\n");
	}
#endif

	/* do the parallel work: create the workers */
	start_time = read_timer();
	for (l = 0; l < numWorkers; l++)
		pthread_create(&workerid[l], &attr, Worker, (void *)l);
	pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
	long myid = (long)arg;
	int total, i, j, first, last;

#ifdef DEBUG
	printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

	// my work

	int max = 0;
	int min = RAND_MAX;

	int maxPosX;
	int maxPosY;

	int minPosX;
	int minPosY;

	// end my work

	/* determine first and last rows of my strip */
	first = myid * stripSize;
	last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

	/* sum values in my strip */
	total = 0;
	for (i = first; i <= last; i++)
		for (j = 0; j < size; j++)
		{
			// my work

			if (matrix[i][j] > max)
			{
				max = matrix[i][j];
				maxPosX = i;
				maxPosY = j;
			}

			if (matrix[i][j] < min)
			{
				min = matrix[i][j];
				minPosX = i;
				minPosY = j;
			}
			// end my work

			total += matrix[i][j];
		}

	// my work
	pthread_mutex_lock(&mutex_localMax);
	localMax[myid][0] = max;
	localMax[myid][1] = maxPosX;
	localMax[myid][2] = maxPosY;
	pthread_mutex_unlock(&mutex_localMax);

	pthread_mutex_lock(&mutex_localMin);
	localMin[myid][0] = min;
	localMin[myid][1] = minPosX;
	localMin[myid][2] = minPosY;
	pthread_mutex_unlock(&mutex_localMin);
	// end my work

	sums[myid] = total;
	Barrier();
	if (myid == 0)
	{
		total = 0;
		for (i = 0; i < numWorkers; i++)
			total += sums[i];
		/* get end time */
		end_time = read_timer();
		/* print results */
		printf("The total is %d\n", total);
		printf("The execution time is %g sec\n", end_time - start_time);

		// my work

		printf("Finding maximum value...\n");

		for (int i = 0; i < numWorkers; i++)
		{
			if (globalMax < localMax[i][0])
			{
				globalMax = localMax[i][0];
				gMaxPosX = localMax[i][1];
				gMaxPosY = localMax[i][2];
			}
		}

		printf("The maximum value is %d, found in array position [%d][%d] \n", globalMax, gMaxPosX, gMaxPosY);

		printf("Finding minimum value...\n");

		for (int i = 0; i < numWorkers; i++)
		{
			if (globalMin > localMin[i][0])
			{
				globalMin = localMin[i][0];
				gMinPosX = localMin[i][1];
				gMinPosY = localMin[i][2];
			}
		}

		printf("The minimum value is %d, found in array position [%d][%d] \n", globalMin, gMinPosX, gMinPosY);

		printf("test case: %d \n", matrix[6][16]);
	}
}
