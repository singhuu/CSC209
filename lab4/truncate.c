#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    Write a function named truncate() that takes a string s and a
    non-negative integer n. If s has more than n characters (not including the
    null terminator), the function should truncate s at n characters and
    return the number of characters that were removed. If s has n or
    fewer characters, s is unchanged and the function returns 0. For example,
    if s is the string "function" and n is 3, then truncate() changes s to
    the string "fun" and returns 5.
*/
int truncate(char *calc_target, int calc_amt)
{
    if (strlen(calc_target) <= calc_amt)
    {
        return 0;
    }
    int temp = strlen(calc_target);
    calc_target[calc_amt] = '\0';
    return (temp-calc_amt);
}

int main(int argc, char **argv)
{
    /* Do not change the main function */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: truncate number string\n");
        exit(1);
    }
    int amt = strtol(argv[1], NULL, 10);

    char *target = argv[2];

    int soln_val = truncate(target, amt);
    printf("%d %s\n", soln_val, target);

    return 0;
}
