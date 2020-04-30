#include <stdio.h>
#include <stdlib.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv)
{

    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }
    FILE *input = fopen(argv[2], "r");
    int tally[BASE] = {0};
    int temp = argc;
    int number_entered;
    int position = strtol(argv[1], NULL, 10);
    if (temp == 2)
    {
        while (!feof(stdin))
        {
            fscanf(stdin, "%d\n", &number_entered);
            add_to_tally(number_entered,position, tally);
        }
    }

    else
    {
        if (!input)
        {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
        while (!feof(input))
        {
            fscanf(input, "%d\n", &number_entered);
            add_to_tally(number_entered, position, tally);
        }
    }
    for (int i = 0; i < BASE; i++)
    {
        printf("%ds: %d\n", i, tally[i]);
    }

    return 0;
}

