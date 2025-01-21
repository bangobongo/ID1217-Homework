/*
    This C program compares 2 text files, and prints all lines that are different between them.

    example usage in terminal:

        cmd:
        diff file1.txt file2.txt

        powershell:
        ./diff file1.txt file2.txt 

    The read of the files is always done with 2 threads, 1 for each file.

    The line comparison is done with MAXWORKERS threads.

    One anomaly observed when testing the speedup, is for small file sizes (a few thousand lines),
    it is faster with 1 thread than several (tested with 2, 4, 8 and 12 threads) this is presumably due
    to extra overhead creating new threads as compared to "just getting on with the work".

    It could also be due to false sharing since we are using a shared array to keep track of the valid_lines array
    in order to keep track of valid lines, since threads might be accessing the same memory blocks in cache.

    I tried allocating the file_x_lines variables both as char ** and char file_x_lines[][] but both seem
    to give this slowdown when using several threads with small to medium files (1 to 500k chars).

    a potential solution might be to force to store the chars in contigious memory from different lines,
    but this is somewhat difficult to do, this should in theory exploit the psoitive effects of the cache and that
    we now compare partitions of the files for each thread. As of right now I believe only the char pointers are
    stored contigiously but not the chars that they then point to, this means mem fragmentation and slower
    access times due to cache misses. But this should cause it to be slow for 1 thread as well, so
    it is probably not the entire solution.

    The chars are now stored in contigious memory, each thread gets its own partition and it is
    still slower with several threads compared to 1. I do not know why anyomore. I think it 
    is unlikely to be false sharing as no 2 threads should share the similar memory space due
    to the thread-wise copied partition.

    After talking to professor Al-Shishtawy, it seemed to him like the slowdown at larger thread counts with
    small problem sizes was due to the associated overhead with creating threads and such. Also 1 thread is
    just that fast at memory compares. 
    
    Running the program with 1 thread on 2 files with 86k lines of 990 chars each, I get comparison time of
    0.062073 sec seconds on an Intel Core i5-1135G7 at 2,42 GHz base speed. 

    Running the program with same files, same CPU but 8 threads, I get a comparison time of 0.040927 seconds.
*/

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

#define MAXWORKERS 12      /* maximum number of workers */
#define MAXLINELENGTH 1000  // Maximum number of chars in a line
#define MAXFILELINES 100000 // Maximum number of lines in a file

typedef struct
{
    int *valid_lines;
    int thread_id;
    int start_line;
    int end_line;
    char *thread_file1_lines;
    char *thread_file2_lines;
} line_compare_data;

double start_time;
double end_time;

char *file_1_lines;
char *file_2_lines;

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

void print_valid_lines(int* valid_lines, int file_length)
{
    for(int i = 0; i < file_length; i++)
    {
        printf("valid lines %d: %d\n", i, valid_lines[i]);
    }
}

void print_diff(int s_file_length, int l_file_length, int s_file, int l_file, char *filename1, char *filename2, int *valid_lines)
{
    // A lot of if statements for neat text formatting, this could be better,
    // but is not the point of the task
    for (int i = 0; i < s_file_length; i++)
    {
        if (!valid_lines[i])
        {
            if (s_file_length - 1 == i && l_file == 1)
            {
                printf("%s line %d: < %s", filename1, i + 1, file_1_lines+(MAXLINELENGTH*i));
                printf("%s line %d: > %s\n", filename2, i + 1, file_2_lines+(MAXLINELENGTH*i));
            }
            else if (s_file_length - 1 == i && l_file == 2)
            {
                printf("%s line %d: < %s\n", filename1, i + 1, file_1_lines+(MAXLINELENGTH*i));
                printf("%s line %d: > %s", filename2, i + 1, file_2_lines+(MAXLINELENGTH*i));
            }
            else
            {
                printf("%s line %d: < %s", filename1, i + 1, file_1_lines+(MAXLINELENGTH*i));
                printf("%s line %d: > %s", filename2, i + 1, file_2_lines+(MAXLINELENGTH*i));
            }
        }
    }

    // if statements for neat text formatting
    for (int i = s_file_length; i < l_file_length; i++)
    {
        if (l_file == 1)
        {
            printf("%s line %d: < %s", filename1, i + 1, file_1_lines+(MAXLINELENGTH*i));
            if (l_file_length - 1 == i)
            {
                printf("\n");
            }
            printf("%s line %d: > \n", filename2, i + 1);
        }
        else if (l_file == 2)
        {
            printf("%s line %d: < \n", filename1, i + 1);
            printf("%s line %d: > %s", filename2, i + 1, file_2_lines+(MAXLINELENGTH*i));
            if (l_file_length - 1 == i)
            {
                printf("\n");
            }
        }
    }
}

int read_lines(char *file_lines, FILE *file)
{
    int number_of_lines = 0;
    char buffer[MAXLINELENGTH];

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        strcpy(file_lines+(number_of_lines++)*MAXLINELENGTH, buffer);
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
    char *file1_partition = data->thread_file1_lines;
    char *file2_partition = data->thread_file2_lines;

    size_t line_size = sizeof(char)*MAXLINELENGTH;

    int line = 0;

    while (line < (end_line-start_line))
    {
        if (memcmp( file1_partition+(line*MAXLINELENGTH), 
                    file2_partition+(line*MAXLINELENGTH), 
                    line_size) != 0)
        {
            valid_lines[line+(start_line)] = 0;
        }
        else
        {
            valid_lines[line+(start_line)] = 1;
        }
        line++;
    }
}

int main(int argc, char *argv[])
{
    pthread_attr_t attr;

    /* set global thread attributes */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    char *filename1 = argv[1];
    char *filename2 = argv[2];

    if(filename1 == NULL || filename2 == NULL)
    {
        printf("Please give 2 files as arguments.");
        return 1;
    }

    size_t max_file_size = MAXFILELINES*MAXLINELENGTH*sizeof(char);

    file_1_lines = malloc(max_file_size);
    file_2_lines = malloc(max_file_size);

    double start_time = read_timer();

    // Creates 2 threads to read one file each
    pthread_create(&threads[0], &attr, read_file_worker, filename1);
    pthread_create(&threads[1], &attr, read_file_worker, filename2);

    // Wait for threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    double end_time = read_timer();
    double read_time = end_time - start_time;

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

        We cant delegate more threads to do work than there are lines in the shortest
        file, therefore the "&& i < shortest_file_length"
    */ 
    for (int i = 0; i < MAXWORKERS && i < shortest_file_length; i++)
    { 
        // simply a thread id for each thread
        int thread_id = i;

        // line where thread should start processing
        int start_line = thread_id*(shortest_file_length/max_threads);

        // line where thread should stop processing, inclusive
        int end_line;

        // if this is the last iteration
        if(!(i+1 < MAXWORKERS && i+1 < shortest_file_length))
        {
            // makes sure last thread processes to the last line
            end_line = shortest_file_length;
        }
        else
        {
            // otherwise divide it evenly
            end_line = (thread_id+1)*(shortest_file_length/max_threads);
        }

        // the size of the partition the thread processes in bytes
        size_t partition_size = sizeof(char)*MAXLINELENGTH*(end_line-start_line);

        // allocate memory for partitions, one for each file
        char *thread_file1_partition = malloc(partition_size);
        char *thread_file2_partition = malloc(partition_size);
        
        // copy memory blocks into thread private memory block
        memcpy( thread_file1_partition, 
                file_1_lines+(sizeof(char)*start_line*MAXLINELENGTH), 
                partition_size);

        memcpy( thread_file2_partition, 
                file_2_lines+(sizeof(char)*start_line*MAXLINELENGTH), 
                partition_size);

        // make the data to send to the thread
        line_compare_data temp_data = { valid_lines,
                                        thread_id, 
                                        start_line, 
                                        end_line,
                                        thread_file1_partition,
                                        thread_file2_partition
                                        };

        // array here to stop threads sharing data in pointers, 
        // trust me it solves a bug :)
        data[i] = temp_data;

        // create the threads, calling line compare function
        pthread_create(&threads[i], &attr, line_compare_worker, &data[i]);
    }

    // wait for all threads to finish
    for (int i = 0; i < MAXWORKERS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    end_time = read_timer();
    double comparison_time = end_time - start_time;

    // Prints the desired ouput for the task
    /* print_diff( shortest_file_length, 
                longest_file_length, 
                shortest_file, 
                longest_file, 
                filename1, 
                filename2, 
                valid_lines
                ); */

    // print_valid_lines(valid_lines, shortest_file_length);
    printf("\n\nCompleted read in %f sec\n", read_time);
    printf("Completed comparison in %f sec\n", comparison_time); 
    
    // Free allocated pointers
    free(file_1_lines);
    free(file_2_lines);
    free(valid_lines);

    return 0;
}