#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

int stackPush(stack_t *s, int i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackPush: stack overflow\n");
        exit(1);
    }
    s->data[s->top] = i;
    return ++s->top;
}
int stackDupPush(stack_t *s, int i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackDupPush: stack overflow\n");
        exit(1);
    }
    if (s->top - i < 0)
    {
        fprintf(stderr, "stackDupPush: reference to non existent stack element\n");
        exit(1);
    }
    // top - i - 1 because top points to a free location
    s->data[s->top] = s->data[s->top-i-1];
    return ++s->top;
}
int stackSwap(stack_t *s, int i)
{
    if (s->top >= STACK_SIZE)
    {
        fprintf(stderr, "stackDupPush: stack overflow\n");
        exit(1);
    }
    if (s->top - i < 0)
    {
        fprintf(stderr, "stackDupPush: reference to non existent stack element\n");
        exit(1);
    }
    // top - i - 1 because top points to a free location
    int temp_top        = s->data[s->top-1];
    s->data[s->top-1]   = s->data[s->top-i-1];
    s->data[s->top-i-1] = temp_top;
    return s->top;
}
int stackPop(stack_t *s)
{
    if (s->top == 0)
    {
        fprintf(stderr, "stackPop: empty stack\n");
        exit(1);
    }
    return s->data[--(s->top)];
}
void stackPrint(const stack_t *s)
{
    int top_copy = s->top;
    printf("\n==================STACK==================\n");
    while ((top_copy--) > 0)
    {
        printf("||\taddress: %d -> content: %d\t||\n", top_copy, s->data[top_copy]);
    }
    printf("===================END===================\n");
}

