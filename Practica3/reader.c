/*
 * Blanca Atienzar Martinez
 * Pau Toni Bibiloni Martinez
 * Unai Pueyo Almodovar
 */

#include <limits.h>

#include "my_lib.h"

int main(int argc, char** argv) {
    // Handle arguments error
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Read stack from file
    struct my_stack* stack = my_stack_read(argv[1]);

    if (!stack) {
        fprintf(stderr, "Error reading stack from file\n");
        return 1;
    }

    printf("Stack length: %d\n", my_stack_len(stack));

    // Calculate min, max and average
    int min = INT_MAX, max = INT_MIN, sum = 0;

    for (int i = 0; i < 10; i++) {
        int* data = (int*) my_stack_pop(stack);
        if (data == NULL) break;

        printf("%d\n", *data);

        if (*data < min) min = *data;
        if (*data > max) max = *data;
        sum += *data;
    }

    printf("\nItems: %d Sum: %d Min: %d Max: %d Average: %d\n",
           10, sum, min, max, sum / 10);

    my_stack_purge(stack);
    return 0;
}