#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#define MAXWORKERS 8  /* maximum number of workers */
#define MAXLINELENGTH 1000 // Maximum number of chars in a line
#define MAXFILELINES 10000 // Maximum number of lines in a file

char** file_1_lines;
char** file_2_lines;

int file_1_number_of_lines;
int file_2_number_of_lines;

pthread_t thread1, thread2;

pthread_mutex_t mutex;
int line_index = 0;

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

int read_lines(char **file_lines, FILE *file) {
    int number_of_lines = 0;
    char buffer[MAXLINELENGTH];

    while (fgets(buffer, sizeof(buffer), file) != NULL) 
    {
        file_lines[number_of_lines++] = strdup(buffer);
    }

    return number_of_lines;
}

void *read_file(void* arg)
{
    char *filename = (char*)arg;
    FILE *file = fopen(filename, "r");

    int number_of_lines;

    if (pthread_equal(thread1, pthread_self())) 
    {
        file_1_number_of_lines = read_lines(file_1_lines, file);
    }
    else 
    {
        file_2_number_of_lines = read_lines(file_2_lines, file);
    }

    fclose(file);
    pthread_exit(NULL);
}

typedef struct {
    int *valid_lines;
    int shortest_file_length;
} line_compare_data;

void *line_compare_worker(void *arg)
{
    line_compare_data *data = (line_compare_data*)arg;
    int *valid_lines = data->valid_lines;
    int min_nr_of_lines = data->shortest_file_length;
    while(1)
    {
        pthread_mutex_lock(&mutex);

        int line;
        if (line_index < min_nr_of_lines) {
            // lines still left to process
            line = line_index++;
            pthread_mutex_unlock(&mutex);
        } else {
            // no lines left
            pthread_mutex_unlock(&mutex);
            break;
        }

        for(int i = 0; i < MAXLINELENGTH; i++)
        {
            if(strcmp(file_1_lines[line], file_2_lines[line]) != 0)
            {
                valid_lines[line] = 0;
            }
            else
            {
                valid_lines[line] = 1;
            }
        }
    }
    

    
}

int main(int argc, char *argv[])
{
    char* filename1 = argv[1];
    char* filename2 = argv[2];

    file_1_lines = malloc(MAXFILELINES*sizeof(char*));
    file_2_lines = malloc(MAXFILELINES*sizeof(char*));

    // Creates 2 threads to read one file each, handles errors
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

    int shortest_file_length, longest_file_length, longest_file;
    if(file_1_number_of_lines < file_2_number_of_lines)
    {
        shortest_file_length = file_1_number_of_lines;
        longest_file_length = file_2_number_of_lines;
        longest_file = 2;
    } 
    else
    {
        shortest_file_length = file_2_number_of_lines;
        longest_file_length = file_1_number_of_lines;
        longest_file = 1;
    }

    int *valid_lines = malloc(shortest_file_length*sizeof(int));

    pthread_t threads[MAXWORKERS];

    // Initialize the mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }

    // Create worker threads
    for (int i = 0; i < MAXWORKERS; i++) 
    {
        line_compare_data data = {valid_lines, shortest_file_length};
        if (pthread_create(&threads[i], NULL, line_compare_worker, &data) != 0) 
        {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAXWORKERS; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&mutex);

    // A lot of if statements for neat text formatting
    for(int i = 0; i < shortest_file_length; i++)
    {
        if(!valid_lines[i])
        {
            if(shortest_file_length-1 == i && longest_file == 1)
            {
                printf("%s line %d: < %s", filename1, i+1, file_1_lines[i]);
                printf("%s line %d: > %s\n", filename2, i+1, file_2_lines[i]);
            }
            else if (shortest_file_length-1 == i && longest_file == 2)
            {
                printf("%s line %d: < %s\n", filename1, i+1, file_1_lines[i]);
                printf("%s line %d: > %s", filename2, i+1, file_2_lines[i]);
            }
            else
            {
                printf("%s line %d: < %s", filename1, i+1, file_1_lines[i]);
                printf("%s line %d: > %s", filename2, i+1, file_2_lines[i]);
            }
            
        }
    }

    // if statements for neat text formatting
    for(int i = shortest_file_length; i < longest_file_length; i++)
    {
        if(longest_file = 1)
        {
            printf("%s line %d: < %s", filename1, i+1, file_1_lines[i]);
            if(longest_file_length-1 == i)
            {
                printf("\n");
            }
            printf("%s line %d: > \n", filename2, i+1);
        }
        else
        {
            printf("%s line %d: < \n", filename1, i+1);
            printf("%s line %d: > %s", filename2, i+1, file_2_lines[i]);
            if(longest_file_length-1 == i)
            {
                printf("\n");
            }
        }
    }
    


    printf("Both files have been read.\n");

    free(file_1_lines);
    free(file_2_lines);
    free(valid_lines);

    return 0;

}
