#define STACK_SIZE 64
#include <stdint.h>

struct _stack
{
    // the stack has ... a stack of data
    intptr_t data[STACK_SIZE];
    // of course, a pointer to the top of the stack
    int32_t top;
};
typedef struct _stack stack_t;

// write the contents of i to the top of the stack
int32_t stackPush(stack_t *s, intptr_t i);
// copy i-th element and push it. 0 is top.
int32_t stackDupPush(stack_t *s, int32_t i);
// swap top with the i-th element.
int32_t stackSwap(stack_t *s, int32_t i);
// remove the top element and returns it. 
intptr_t stackPop(stack_t *s);
// prints the stack contents for inspection
void stackPrint(const stack_t *s);

