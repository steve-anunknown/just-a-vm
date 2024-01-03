#include "stack.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// one could perform checks here to ensure the integrity of the stack.
// but it is undefined behaviour.

inline int32_t stackPush(stack_t *s, uintptr_t i)
{
    s->data[s->top] = i;
    return ++(s->top);
}
inline int32_t stackDupPush(stack_t *s, int32_t i)
{
    // top - i - 1 because top points to a free location
    s->data[s->top] = s->data[s->top-i-1];
    return ++(s->top);
}
inline int32_t stackSwap(stack_t *s, int32_t i)
{
    const uintptr_t temp_top   = s->data[s->top-1];
    s->data[s->top-1]   = s->data[s->top-i-1];
    s->data[s->top-i-1] = temp_top;
    return s->top;
}
void stackPrint(const stack_t *s)
{
    int32_t top_copy = s->top;
    printf("\n=======================STACK=======================\n");
    while ((top_copy--) > 0)
    {
        // this prints the contents in hex form.
        printf("||\taddress: %d -> content: %lx\t||\n", top_copy, s->data[top_copy]);
    }
    printf("========================END========================\n");
}

