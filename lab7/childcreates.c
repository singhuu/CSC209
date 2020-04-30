#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;
    int iterations;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: forkloop <iterations>\n");
        exit(1);
    }

    iterations = strtol(argv[1], NULL, 10);
    int check;
    int n = 0;
    for (i = 0; i < iterations; i++)
    {
        if (n < 1)
        {
            n = fork();
            wait(&check);
        }
        if (n < 0)
        {
            perror("fork");
            exit(1);
        }
        printf("ppid = %d, pid = %d, i = %d\n", getppid(), getpid(), i);
    }

    return 0;
}

