#ifndef GC_H
#define GC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "stack.h"
#include "cons.h"
#include "bitarray.h"
#include "utils.h"

struct garbage_collector
{
    stack_t* machine;
    uint32_t* heap;
    size_t size;
    uintptr_t bottom;
    uint32_t *bitarray;
    cons* freelist;
};
typedef struct garbage_collector garbage_collector;

bool markAndSweep(garbage_collector* gc);

#endif
