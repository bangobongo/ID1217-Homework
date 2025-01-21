
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


void print_valid_lines(int* valid_lines, int file_length);
void print_diff(int s_file_length, int l_file_length, int s_file, int l_file, char *filename1, char *filename2, int *valid_lines);

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