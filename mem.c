#include <stdlib.h>
#include <stdio.h>
#include "mem.h"


void * reallocate (void *array, size_t new_size)
{
        array = realloc (array, new_size);

        if (!array) {
                perror ("realloc");
                exit (EXIT_FAILURE);
        }
        return array;
}

void *allocate (size_t size)
{
        void *mem = malloc (size);
        if (!mem) {
                perror ("malloc");
                exit (EXIT_FAILURE);
        }

        return mem;
}