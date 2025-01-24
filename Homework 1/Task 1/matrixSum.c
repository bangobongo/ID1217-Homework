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
#define MAXWORKERS 8  /* maximum number of workers */
#define MODULO 99

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

long myid[MAXWORKERS];

long long unsigned total; // big datatype to prevent overflow

int counter = 0; // represents next unprocessed index in matrix

// declare mutex for interacting with the bag of tasks
pthread_mutex_t index_mutex;

// declare mutex for adding to the shared sums variable
pthread_mutex_t sums_mutex;

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
// int sums[MAXWORKERS];		  /* partial sums */ - removed for part b
unsigned long long sums = 0;	/* replaces the array */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

// my work - local max/mins and pos
int localMax[MAXSIZE][3];
int localMin[MAXSIZE][3];

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[])
{
	// initialize mutex for interacting with bag of tasks
	pthread_mutex_init(&index_mutex, NULL);

	// initialize mutex for interacting with shared sums variable
	pthread_mutex_init(&sums_mutex, NULL);

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
			matrix[i][j] = rand() % MODULO;
		}
	}

	// testing
	// matrix[6][16] = 150;

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
	{
		myid[l] = l;
		pthread_create(&workerid[l], &attr, Worker, (void *)&myid[l]);
	}
	// terminating the threads to avoid using barrier function for b
	for (i = 0; i < numWorkers; i++)
		pthread_join(workerid[i], NULL);

	/* get end time */
	end_time = read_timer();
	/* print results, no need to use mutex because all the threads are done so no risk of concurrency issues */
	printf("The total is %llu\n", sums); // using %llu instead of %d because sums is long long unsigned and not int
	printf("The execution time is %g sec\n", end_time - start_time);

	/* find the global maximums and minimums by iterating through the localMax
	array and using the coordinates to record the positions of these */
	for (int i = 0; i < size; i++)
	{
		if (globalMax < localMax[i][0])
		{
			globalMax = localMax[i][0];
			gMaxPosX = localMax[i][1];
			gMaxPosY = localMax[i][2];
		}

		if (globalMin > localMin[i][0])
		{
			globalMin = localMin[i][0];
			gMinPosX = localMin[i][1];
			gMinPosY = localMin[i][2];
		}
	}

	/* because the matrix is massive, but only selects from a small pool of numbers (0 to 98, which was the suggested range),
	inevitably the largest and smallest numbers will occur many times across the whole matrix, so the results will always be 
	[something][0], because the logic only updates the global maximum and minimum when a strictly larger or smaller value is 
	found. once the first occurrence of the maximum (98) or minimum (0) is encountered, subsequent occurences are not saved
	because the < and > operators used in the comparison. you can see more variability in the results of the positions
	by increasing the modulo and decreasing the max size definitions. however, using too large arrays and modulos will cause
	the sum to overflow if it exceeds 18 446 744 073 709 551 615 */

	printf("The maximum value is %d, found in array position [%d][%d] \n", globalMax, gMaxPosX, gMaxPosY);

	printf("The minimum value is %d, found in array position [%d][%d] \n", globalMin, gMinPosX, gMinPosY);

	//printf("test case: %d \n", matrix[6][16]);

	return 0;
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg)
{
	//long myid = (long)arg; <- this line causes a segmentation fault
	/* because its taking the argument (the memory address passed
	to the thread function) and casting it to a long instead of reading the
	integer stored at that address. instead, we do: */

	long myid = *((long *)arg); // read the integer at that address

	/* which takes the argument, casts it to a long pointer, then deferences that
	and saves it in myid */

	// int i, j, first, last;

#ifdef DEBUG
	printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

	while (counter < size)
	{
		// collect task from bag of tasks
		pthread_mutex_lock(&index_mutex);
		/* make sure the counter hasnt passed size,
		which can happen if two threads enter the while loop
		at the same time. without this condition, both threads
		will think theres still a row left to process*/
		if (counter >= size) {
			/* if the counter is bigger than size, 
			release the lock and leave the loop*/
			pthread_mutex_unlock(&index_mutex);
			break;
		}
		/* take a row/index from the matrix using a pointer */
		int *myTask = matrix[counter];
		/* save the current value of the counter *before* incrementing it,
		or the thread will work on the succeeding index rather than the 
		current one*/
		int localCounter = counter;
		counter++;
		/* once all necessary operations to do with the counter or matrix
		are done, we release the mutex */
		pthread_mutex_unlock(&index_mutex);

		/* declare and initialize various variables to do with
		the maximum and minimum values, as well as their locations in 
		the matrix and the sum of all values in this row */
		int max = 0;
		int min = RAND_MAX;

		int maxPosX = 0;
		int minPosX = 0;

		int localTotal = 0;

		/* iterate over the row, reassign maximum and minimum and their positions
		if necessary and add to the sum. */
		for (int i = 0; i < size; i++)
		{
			if (myTask[i] > max)
			{
				max = myTask[i];
				maxPosX = i;
			}

			if (myTask[i] < min)
			{
				min = myTask[i];
				minPosX = i;
			}

			// printf("matrix state: %d \n", matrix[i][j]);
			// printf("running total: %d \n", total);
			localTotal += myTask[i];
		}

		/* change the localMax and localMin arrays to reflect the biggest
		and smallest numbers found in this row/index, as well as their positions.
		these are used by the main thread to determine what the global maximum and minimum
		values are */
		localMax[localCounter][0] = max;
		localMax[localCounter][1] = maxPosX;

		localMin[localCounter][0] = min;
		localMin[localCounter][1] = minPosX;

		/* localmax[localcounter][2] and localmin[localcounter][2] are always set to localcounter 
        because they represent the y-coordinate (row index) of the matrix. in this implementation, 
        the thread processes one row at a time, and localcounter keeps track of which specific row 
        is being processed. this makes localcounter inherently the y-coordinate of the maximum and 
        minimum values found in that row. in contrast, maxposx and minposx are necessary to track the column 
        indices (x-coordinates) of the maximum and minimum values within the row, since these 
        can vary depending on where the values are found. */
		localMax[localCounter][2] = localCounter;
		localMin[localCounter][2] = localCounter;
		

		// printf("total: %d \n", total);
		/* add the partial sum to the shared sums variable */
		pthread_mutex_lock(&sums_mutex);
		sums += localTotal;
		pthread_mutex_unlock(&sums_mutex);

		// Barrier(); do not use the barrier for b
	}
}