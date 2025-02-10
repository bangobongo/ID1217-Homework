

/*
    queens.c

    gives the number of possible solutions for solving the N-queens problem
    with N = BOARD_SIZE defined below, if you want to change the N, then change
    that value (BOARD_SIZE) and recompile. Same goes for number of threads, set NUM_THREADS 
    before compiling and recompile to use more or less threads.

    compile using:

        gcc -fopenmp -o queens queens.c

    run using:
        cmd:        queens
        powershell: ./queens

    For the results of the runs, we did runs using a 8x8 board,
    these runs were so fast that the OMP timer could not measure
    the time with a high enough resolution, it just resulted 
    in 0.000 seconds for all numbers of threads.

    Therefore, we decided to do our runs with a larger board size,
    13x13 as this gave a reasonable time for each run that was still
    measurable.

    +----------------+----------+--------+
    | Board size(NxN)| Threads  | Time  |
    +----------------+----------+--------+
    | 13            | 1         | 0.959  |
    | 13            | 1         | 0.928  |
    | 13            | 1         | 0.942  |
    | 13            | 1         | 0.829  |
    | 13            | 1         | 0.832  |
    | 13            | 2         | 0.658  |
    | 13            | 2         | 0.593  |
    | 13            | 2         | 0.627  |
    | 13            | 2         | 0.528  |
    | 13            | 2         | 0.531  |
    | 13            | 4         | 0.407  |
    | 13            | 4         | 0.396  |
    | 13            | 4         | 0.470  |
    | 13            | 4         | 0.436  |
    | 13            | 4         | 0.393  |
    | 13            | 8         | 0.282  |
    | 13            | 8         | 0.299  |
    | 13            | 8         | 0.299  |
    | 13            | 8         | 0.344  |
    | 13            | 8         | 0.299  |
    +---------------+-----------+--------+

    For 1 thread (sequential) the median time was 0.928.

    For 2 threads, the median time was 0.593 so speedup of 1.565.
    For 4 threads, the median time was 0.407 so speedup of 2.280.
    For 2 threads, the median time was 0.299 so speedup of 3.104.

    We use (median sequential time / median parallel time) to get speedup.

    We also made the assumption that "generating all solutions" does not mean
    that they need to be displayed later, just generated so that it increments a
    counter, so we know how many solutions there are.
*/


#include <stdlib.h>
#include <stdio.h>

#include <omp.h>

#define BOARD_SIZE 13
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

    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for(int row = 0; row < BOARD_SIZE; row++)
    {
        pos_state thread_board[BOARD_SIZE][BOARD_SIZE];
        boardcpy(thread_board, board);
        place_queen(thread_board, row, 0);
        recursive_solve(thread_board, 1);
    }
    double end_time = omp_get_wtime();

    printf("Completed in %f seconds on %d threads\n", end_time-start_time, NUM_THREADS);
    printf("%d solutions found for %dx%d board\n", nr_of_solutions, BOARD_SIZE, BOARD_SIZE);

    return 0;
}