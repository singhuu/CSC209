#include <stdio.h>

#include "benford_helpers.h"

int count_digits(int num)
{
    if (num == 0)
    {
        return 1;
    }
    int temp = 0;
    while (num != 0)
    {
        num = num / BASE;
        temp++;
    }
    return temp;
}

int get_ith_from_right(int num, int i)
{
    for (int j = 0; j < i; j++)
    {
        num = num / BASE;
    }
    return (num % BASE);
}

int get_ith_from_left(int num, int i)
{
    int convert_to_right_index = count_digits(num) - i - 1;
    return get_ith_from_right(num, convert_to_right_index);
}

void add_to_tally(int num, int i, int *tally)
{
    int temp2 = get_ith_from_left(num, i);
    tally[temp2]++;
}

