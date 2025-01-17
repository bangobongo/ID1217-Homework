#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXWORKERS 8  /* maximum number of workers */
#define MAXFILELINES 10000 // Maximum number of lines in a file

char* file_1_lines[MAXFILELINES];
char* file_2_lines[MAXFILELINES];

pthread_t thread1, thread2;

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

void *read_file(void* arg)
{
    char *filename = (char*)arg;
    FILE *file = fopen(filename, "r");

    printf("Reading file: %s\n", filename);

    int buffer_size = 2048;

    char buffer[buffer_size];

    int number_of_lines = 0;
    char **lines;

    if(pthread_equal(thread1, pthread_self()))
    {
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            file_1_lines[number_of_lines++] = buffer;
        }
    }
    else if (pthread_equal(thread2, pthread_self()))
    {
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            file_2_lines[number_of_lines++] = buffer;
        }
    }

    

    

    

    printf("%d lines in %s\n", number_of_lines, filename);

    fclose(file);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    char* filename1 = argv[1];
    char* filename2 = argv[2];

    printf("Starting...\n");

    if(pthread_create( &thread1, NULL, read_file, filename1) != 0)
    {
        perror("Failed to create thread 1");
        return 1;
    }
    if(pthread_create( &thread2, NULL, read_file, filename2) != 0)
    {
        perror("Failed to create thread 2");
        return 1;
    }


    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Both files have been read.\n");
    return 0;

}
