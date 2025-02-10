

/*
    queens.c

    gives the number of possible solutions for solving the N-queens problem
    with N = BOARD_SIZE defined below, if you want to change the N, then change
    that value and recompile. Same goes for number of threads, set NUM_THREADS 
    before compiling.

    compile using:

        gcc -fopenmp -Og -o queens queens.c

    the -Og is to set the compiler into "optimize debug experience" it is not strictly needed.

    run using:
        cmd:        queens
        powershell: ./queens
*/


#include <stdlib.h>
#include <stdio.h>

#include <omp.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>

#define BOARD_SIZE 8
#define NUM_THREADS 8

typedef enum {
    EMPTY,
    QUEEN
} pos_state;

int nr_of_solutions = 0;

void empty_board(pos_state board[BOARD_SIZE][BOARD_SIZE])
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }
}

void print_board(pos_state board[BOARD_SIZE][BOARD_SIZE])
{
    pos_state current_pos;
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            current_pos = board[i][j];
            if(current_pos == EMPTY)
            {
                printf("[ ]");
            }
            else
            {
                printf("[Q]");
            }
            
        }
        printf("\n");
    }
    printf("\n");
}

void place_queen(pos_state board[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    board[row][col] = QUEEN;
}

void remove_queen(pos_state board[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    board[row][col] = EMPTY;
}

int is_valid_placement(pos_state board[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    // check diagonal down left
    for(int i = 1; row+i < BOARD_SIZE && col-i >= 0; i++)
    {
        if(board[row+i][col-i] == QUEEN)
        {
            return 0;
        }
    }
    // check diagonal up left
    for(int i = 1; row-i >= 0 && col-i >= 0; i++)
    {
        if(board[row-i][col-i] == QUEEN)
        {
            return 0;
        }
    }
    // check to the left
    for(int i = 0; i < col; i++)
    {
        if(board[row][i])
        {
            return 0;
        }
    }
    return 1;
}

void boardcpy(pos_state dest[BOARD_SIZE][BOARD_SIZE], pos_state src[BOARD_SIZE][BOARD_SIZE])
{
    for(int i = 0; i < BOARD_SIZE; i++)
    {
        for(int j = 0; j < BOARD_SIZE; j++)
        {
            dest[i][j] = src[i][j];
        }
    }
}

void recursive_solve(pos_state board[BOARD_SIZE][BOARD_SIZE], int col)
{
    if(col == BOARD_SIZE)
    {
        #pragma omp critical
        nr_of_solutions++;
        return;
    }

    for(int row = 0; row < BOARD_SIZE; row++)
    {
        if(is_valid_placement(board, row, col))
        {
            place_queen(board, row, col);
            recursive_solve(board, col+1);
            remove_queen(board, row, col);
        }
    }
}

int main()
{   
    omp_set_num_threads(NUM_THREADS);
    
    pos_state board[BOARD_SIZE][BOARD_SIZE];
    empty_board(board);

    double start_time = read_timer();

    #pragma omp parallel for
    for(int row = 0; row < BOARD_SIZE; row++)
    {
        pos_state thread_board[BOARD_SIZE][BOARD_SIZE];
        boardcpy(thread_board, board);
        place_queen(thread_board, row, 0);
        recursive_solve(thread_board, 1);
    }
    double end_time = read_timer();

    printf("Completed in %f\n", end_time-start_time);
    printf("%d solutions found for %dx%d board\n", nr_of_solutions, BOARD_SIZE, BOARD_SIZE);

    return 0;
}