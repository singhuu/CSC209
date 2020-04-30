#include <stdio.h>
#include <stdlib.h>

#include "life2D_helpers.h"

// Creating a new helper method for main()
void read(int *array, int count){
    while(!feof(stdin)) {
        fscanf(stdin, "%d ", array + count);
        count ++;
    }
}
int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    // Three inputs are height, width and number of states defined here
    int h = strtol(argv[1], NULL, 10);
    int w = strtol(argv[2], NULL, 10);
    int num_of_states = strtol(argv[3], NULL, 10);
    int product = h * w;
    int count = 0;
    // Initializing Array that stores the positions of 0 and 1
    int *array = (int *) malloc(sizeof(int)*product);
    read(array, count);
    for(int i = 0; i < num_of_states; i++){
        print_state(array, h, w);
        update_state(array, h, w);
    }
    return 0;
}
    

