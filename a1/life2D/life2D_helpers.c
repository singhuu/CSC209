#include <stdio.h>
#include <stdlib.h>

void print_state(int *board, int num_rows, int num_cols)
{
    for (int i = 0; i < num_rows * num_cols; i++)
    {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}
int copy_board(int *board, int *temp_board, int product)
{
    for (int i = 0; i < product; i++)
    {
        temp_board[i] = board[i];
    }
    return *temp_board;
}

int counter(int *board, int i, int c, int num)
{
    int temp = 0;
    if (board[i + num] == 1)
        temp++;
    // checking for every case
    for (int j = -1; j <= 1; j++)
    {
        if (board[i + num * (c) + j] == 1)
            temp++;
    }
    return temp;
}
void update_state(int *board, int num_rows, int num_cols)
{
    /* Copy pasting the rules for my reference -
    source: http://web.mit.edu/sp.268/www/2010/lifeSlides.pdf
    The rules of the game are simple, and describe the evolution of the
    grid:
     Birth: a cell that is dead at time t will be alive at time t + 1
    if exactly 3 of its eight neighbors were alive at time t.
     Death: a cell can die by:
    -> Overcrowding: if a cell is alive at time t + 1 and 4 or more of
    its neighbors are also alive at time t, the cell will be dead at
    time t + 1.
    -> Exposure: If a live cell at time t has only 1 live neighbor or no
    live neighbors, it will be dead at time t + 1.
     Survival: a cell survives from time t to time t + 1 if and only
    if 2 or 3 of its neighbors are alive at time t.
    */
    int product = num_rows * num_cols;
    int *temp_board = (int *)malloc(sizeof(int) * product);
    copy_board(board, temp_board, product);

    // Applying the rules and making changes here
    int c = num_cols;
    int r = num_rows;
    int count;
    for (int i = c; i < (r * c - c); i++)
    {
        // Taking a range of possible cases here
        if ((i + 1) % c !=0 && (i+1) % c !=  1)
        {
            count = counter(board, i, c, -1) + counter(board, i, c, 1);
            if (count < 2 || count > 3)
            {
                temp_board[i] = 0;
            }
            else
            {
                temp_board[i] = 1;
            }
        }
    }

    copy_board(temp_board, board, product);
}

