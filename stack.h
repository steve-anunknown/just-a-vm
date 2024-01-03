#include <stdint.h>
#ifndef STACK_H
#define STACK_H
#define STACK_SIZE 256

struct stack
{
    /* 
     * The stack has ... a stack of data.
     * The data can be integers and addresses
     * therefore we need a type that can
     * switch between representing any of them.
     *
     * In order for the garbage collection to function,
     * 2 bits seem to be needed. One to signify whether
     * the content of the stack is a pointer to the 
     * heap and another to signify whether it is to
     * be freed.
     */
    uintptr_t data[STACK_SIZE];
    /*
     * Of course, a pointer to the (free) top of the stack
     * the pointerW is to be used as an index.
     * I think it does not make much of a difference
     * whether uint32_t or int32_t is used, since both
     * of them wrap around eventually.
     */
    int32_t top;
};
typedef struct stack stack_t;

// write the contents of i to the top of the stack
int32_t stackPush(stack_t *s, uintptr_t i);
// copy i-th element and push it. 0 is top.
int32_t stackDupPush(stack_t *s, int32_t i);
// swap top with the i-th element.
int32_t stackSwap(stack_t *s, int32_t i);
// prints the stack contents for inspection
void stackPrint(const stack_t *s);

#endif
