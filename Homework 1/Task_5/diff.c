/*
    This C program compares 2 text files, and prints all lines that are different between them.

    example usage in terminal:
        diff file1.txt file2.txt

    The read of the files is always done with 2 threads, 1 for each file.

    The line comparison is done with MAXWORKERS threads.

    One anomaly observed when testing the speedup, is for small file sizes (a few thousand lines),
    it is faster with 1 thread than several (tested with 4, 8 and 12 threads) this is presumably due
    to extra overhead creating new threads as compared to "just getting on with the work".

    It could also be due to false sharing since we are using a shared array to keep track of the valid_lines array
    in order to keep track of valid lines, since threads might be accessing the same memory blocks in cache.

*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#define MAXWORKERS 12      /* maximum number of workers */
#define MAXLINELENGTH 1000  // Maximum number of chars in a line
#define MAXFILELINES 100000 // Maximum number of lines in a file

typedef struct
{
    int *valid_lines;
    int thread_id;
    int start_line;
    int end_line;
} line_compare_data;

double start_time;
double end_time;

char **file_1_lines;
char **file_2_lines;

int file_1_number_of_lines;
int file_2_number_of_lines;

pthread_t threads[MAXWORKERS];

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

int read_lines(char **file_lines, FILE *file)
{
    int number_of_lines = 0;
    char buffer[MAXLINELENGTH];

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        file_lines[number_of_lines++] = strdup(buffer);
    }

    return number_of_lines;
}

void *read_file_worker(void *arg)
{
    char *filename = (char *)arg;
    FILE *file = fopen(filename, "r");

    int number_of_lines;

    if (pthread_equal(threads[0], pthread_self()))
    {
        file_1_number_of_lines = read_lines(file_1_lines, file);
    }
    else
    {
        file_2_number_of_lines = read_lines(file_2_lines, file);
    }
    fclose(file);
}

void *line_compare_worker(void *arg)
{
    line_compare_data *data = (line_compare_data *)arg;
    int *valid_lines = data->valid_lines;
    int thread_id = data->thread_id;
    int start_line = data->start_line;
    int end_line = data->end_line;

    int line = start_line;

    while (line < end_line)
    {
        if (strcmp(file_1_lines[line], file_2_lines[line]) != 0)
        {
            valid_lines[line] = 0;
        }
        else
        {
            valid_lines[line] = 1;
        }
        line++;
    }
}

int main(int argc, char *argv[])
{
    char *filename1 = argv[1];
    char *filename2 = argv[2];

    file_1_lines = malloc(MAXFILELINES * sizeof(char *));
    file_2_lines = malloc(MAXFILELINES * sizeof(char *));

    double start_time = read_timer();

    // Creates 2 threads to read one file each
    pthread_create(&threads[0], NULL, read_file_worker, filename1);
    pthread_create(&threads[1], NULL, read_file_worker, filename2);

    // Wait for threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    double end_time = read_timer();

    printf("Completed read in %f sec\n", end_time - start_time);

    int shortest_file_length, longest_file_length, longest_file, shortest_file;

    if (file_1_number_of_lines < file_2_number_of_lines)
    {
        shortest_file_length = file_1_number_of_lines;
        longest_file_length = file_2_number_of_lines;
        longest_file = 2;
        shortest_file = 1;
    }
    else if (file_1_number_of_lines > file_2_number_of_lines)
    {
        shortest_file_length = file_2_number_of_lines;
        longest_file_length = file_1_number_of_lines;
        longest_file = 1;
        shortest_file = 2;
    }
    else
    {
        shortest_file_length = file_1_number_of_lines;
        longest_file_length = file_2_number_of_lines;
        longest_file = 2;
        shortest_file = 1;
    }

    int max_threads;
    if(shortest_file_length<MAXWORKERS)
    {
        max_threads = shortest_file_length;
    }
    else
    {
        max_threads = MAXWORKERS;
    }

    int *valid_lines = malloc(shortest_file_length * sizeof(int));

    // Fixed bug of shared memory addresses
    line_compare_data data[max_threads];

    start_time = read_timer();

    /*
        Create worker threads

        The main thread delegates partitions of the file lines to compare
        in order to maximize spatial locality of cache. A bag of tasks approach
        was tried, but that was marginally slower. We still have some sort of
        false sharing problem as more threads is slower for small files.
    */ 
    for (int i = 0; i < MAXWORKERS && i < shortest_file_length; i++)
    { 
        int thread_id = i;
        int start_line = thread_id*(shortest_file_length/max_threads);
        int end_line;

        // if this is the last iteration
        if(!(i+1 < MAXWORKERS && i+1 < shortest_file_length))
        {
            end_line = shortest_file_length;
        }
        else
        {
            end_line = (thread_id+1)*(shortest_file_length/max_threads);
        }
        
        line_compare_data temp_data = {valid_lines, thread_id, start_line, end_line};
        data[i] = temp_data;
        pthread_create(&threads[i], NULL, line_compare_worker, &data[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < MAXWORKERS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    end_time = read_timer();
    double comparison_time = end_time - start_time;
    printf("Completed comparison in %f sec\n", comparison_time);

    // A lot of if statements for neat text formatting, this could be better,
    // but is not the point of the task
    for (int i = 0; i < shortest_file_length; i++)
    {
        if (!valid_lines[i])
        {
            if (shortest_file_length - 1 == i && longest_file == 1)
            {
                printf("%s line %d: < %s", filename1, i + 1, file_1_lines[i]);
                printf("%s line %d: > %s\n", filename2, i + 1, file_2_lines[i]);
            }
            else if (shortest_file_length - 1 == i && longest_file == 2)
            {
                printf("%s line %d: < %s\n", filename1, i + 1, file_1_lines[i]);
                printf("%s line %d: > %s", filename2, i + 1, file_2_lines[i]);
            }
            else
            {
                printf("%s line %d: < %s", filename1, i + 1, file_1_lines[i]);
                printf("%s line %d: > %s", filename2, i + 1, file_2_lines[i]);
            }
        }
    }

    // if statements for neat text formatting
    for (int i = shortest_file_length; i < longest_file_length; i++)
    {
        if (longest_file == 1)
        {
            printf("%s line %d: < %s", filename1, i + 1, file_1_lines[i]);
            if (longest_file_length - 1 == i)
            {
                printf("\n");
            }
            printf("%s line %d: > \n", filename2, i + 1);
        }
        else if (longest_file == 2)
        {
            printf("%s line %d: < \n", filename1, i + 1);
            printf("%s line %d: > %s", filename2, i + 1, file_2_lines[i]);
            if (longest_file_length - 1 == i)
            {
                printf("\n");
            }
        }
    }

    for(int i = 0; i < shortest_file_length; i++)
    {
        printf("\n valid lines %d: %d", i, valid_lines[i]);
    }

    // Free allocated pointers
    free(file_1_lines);
    free(file_2_lines);
    free(valid_lines);

    return 0;
}