#include "stack.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int32_t stackPush(stack_t *s, uintptr_t i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackPush: stack overflow\n");
        exit(1);
    }
    s->data[s->top] = i;
    return ++(s->top);
}
int32_t stackDupPush(stack_t *s, int32_t i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackDupPush: stack overflow\n");
        exit(1);
    }
    if (i < 0 || s->top - 1 - i < 0)
    {
        fprintf(stderr, "stackDupPush: reference to non existent stack element\n");
        exit(1);
    }
    // top - i - 1 because top points to a free location
    s->data[s->top] = s->data[s->top-i-1];
    return ++(s->top);
}
int32_t stackSwap(stack_t *s, int32_t i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackDupPush: stack overflow\n");
        exit(1);
    }
    if (i < 0 || s->top - 1 - i < 0)
    {
        fprintf(stderr, "stackDupPush: reference to non existent stack element\n");
        exit(1);
    }
    // top - i - 1 because top points to a free location
    const uintptr_t temp_top   = s->data[s->top-1];
    s->data[s->top-1]   = s->data[s->top-i-1];
    s->data[s->top-i-1] = temp_top;
    return s->top;
}
uintptr_t stackPop(stack_t *s)
{
    if (s->top == 0)
    {
        fprintf(stderr, "stackPop: empty stack\n");
        exit(1);
    }
    s->top = s->top - 1;
    return s->data[s->top];
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

