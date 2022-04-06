#ifndef mem_h
#define mem_h

#include <stdlib.h>

void *reallocate (void *array, size_t new_size);
void *allocate (size_t size);

#endif