#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"

/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount)
{
    int m, child3b, child3c, result, i, j;
    double l_arr, r_arr, min_of_leftandright;
    double left, right, fin_min;

    // Step 1 of the algorithm
    if (pdmax == 0 || n < 4)
    {
        return closest_serial(p, n);
    }
    // Step 2: Split the Array into two
    m = n / 2;
    // Step 3.a: Create two child processes to solve left and right halves of the algorithm
    int pipe1[2];
    if (pipe(pipe1) == -1) // pipe(pipe1) == -1
    {
        perror("Parent and Child 1");
        exit(1);
    }
    // Step 3.b : Forking a child and subsequent steps
    child3b = fork();
    if (child3b < 0)
    {
        perror("Fork");
        exit(1);
    }
    else if (child3b > 0)
    {
        if (close(pipe1[1]) == -1) // Ensuring Error Checking, marks were cut last time
        {
            perror("Parent");
            exit(1);
        }
    }
    else
    {
        // Performing algorithm. NOTE TO SELF : Maintain the opening and closing order
        
        if (close(pipe1[0]) == -1)
        {
            perror("Child : Reading Closed");
            exit(1);
        }
        // Comparing if the size is the same or not using sizeof()
        left = closest_parallel(p, m, (pdmax - 1), pcount);
        if (write(pipe1[1], &left, sizeof(double)) != sizeof(double))
        {
            perror("Parent : Writing");
            exit(1);
        }
        // Checking error while closing
        if (close(pipe1[1]) == -1)
        {
            perror("Child : Writing");
            exit(1);
        }
        // Exit with status equal to the number of worker processes
        // in the process tree rooted at the current process
        exit(*pcount + 1);
    }

    // Step 3.c & d : Following the same drill as 3.b
    int pipe2[2];
    if (pipe(pipe2) == -1) // pipe(pipe1) == -1
    {
        perror("Parent and Child 2");
        exit(1);
    }
    child3c = fork();
    if (child3c < 0)
    {
        perror("Fork");
        exit(1);
    }
    else if (child3c > 0)
    {
        if (close(pipe2[1]) == -1) // Ensuring Error Checking, marks were cut last time
        {
            perror("Parent");
            exit(1);
        }
    }
    else
    {
        // Performing algorithm. NOTE TO SELF : Maintain the opening and closing order
        right = closest_parallel(p+m, n-m, pdmax-1,pcount);
        if (close(pipe2[0]) == -1)
        {
            perror("Child : Reading Closed");
            exit(1);
        }
        // Comparing if the size is the same or not using sizeof()
        if (write(pipe2[1], &right, sizeof(double)) != sizeof(double))
        {
            perror("Parent : Writing");
            exit(1);
        }
        // Checking error while closing
        if (close(pipe2[1]) == -1)
        {
            perror("Child : Writing");
            exit(1);
        }
        // Exit with status equal to the number of worker processes
        // in the process tree rooted at the current process
        exit(*pcount + 1);
    }

    // Step 3.4 : Wait for the two child processes to complete
    if (waitpid(child3b, &result, 0) == -1)
    {
        perror("Child 1");
        exit(1);
    }
    *pcount += WEXITSTATUS(result);
    // Similarly, with the second child
    if (waitpid(child3c, &result, 0) == -1)
    {
        perror("Child 1");
        exit(1);
    }
    *pcount += WEXITSTATUS(result);

    // Step 5 : Read from the Pipes NOTETOSELF: First READ, then CLOSE
    if (read(pipe1[0], &l_arr, sizeof(double)) != sizeof(double))
    {
        perror("Child : Reading");
        exit(1);
    }
    if (close(pipe1[0]) == -1)
    {
        perror("Parent : Unable to Close");
        exit(1);
    }
    if (read(pipe2[0], &r_arr, sizeof(double)) != sizeof(double))
    {
        perror("Child : Reading");
        exit(1);
    }
    if (close(pipe2[0]) == -1)
    {
        perror("Parent : Unable to Close");
        exit(1);
    }
    // Step 6 : Refer to serial_closest.c and use divide and conquer to determine
    // the distance between the closest pair of points, with distance is smaller than d

    // We find the min of the left array and the right array
    min_of_leftandright = min(r_arr, l_arr);
    // We can build a struct that contains all the points closer than d
    struct Point *closer = malloc(n * (sizeof(struct Point)));
    if (closer == NULL)
    {
        perror("*closer initialization issues, see: malloc allocation");
        exit(1);
    }

    // Step 7 : We compare each case from the struct now
    struct Point mid_struct = p[m];
    i = 0;
    for (j = 0; j < n; j++)
    {
        int val = abs(mid_struct.x - p[j].x);
        if (min_of_leftandright > val)
        {
            closer[i] = p[j];
            i++;
        }
    }
    // Finally we find the final min and free the stucts
    fin_min = min(min_of_leftandright, strip_closest(closer, i, min_of_leftandright));
    free(closer);

    return fin_min;
}
