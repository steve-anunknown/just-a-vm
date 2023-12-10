#define STACK_SIZE 64

struct _stack
{
    // the stack has ... a stack of data
    int data[STACK_SIZE];
    // of course, a pointer to the top of the stack
    int top;
};
typedef struct _stack stack_t;

// write the contents of i to the top of the stack
int stackPush(stack_t *s, int i);
// copy i-th element and push it. 0 is top.
int stackDupPush(stack_t *s, int i);
// swap top with the i-th element.
int stackSwap(stack_t *s, int i);
// remove the top element and returns it. 
int stackPop(stack_t *s);
// prints the stack contents for inspection
void stackPrint(const stack_t *s);

