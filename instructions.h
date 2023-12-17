#define HALT 0X00

#define JUMP 0X01       // jump to address (2 bytes).
#define SIZEOF_JUMP 3
#define JNZ 0X02        // pop, and jump to address (2 bytes) 
                        // if popped element not zero.
#define SIZEOF_JNZ 3
#define DUP 0X03        // push i-th (1 unsigned byte) element to the top of the stack.
#define SIZEOF_DUP 2
#define SWAP 0X04       // swap top of the stack and i-th (1 unsigned byte) element.
#define SIZEOF_SWAP 2
#define DROP 0X05       // pop and ignore.
#define SIZEOF_DROP 1
/* PUSH OPERATORS */
#define PUSH4 0X06      // push 4byte (signed) value.
#define SIZEOF_PUSH4 5
#define PUSH2 0X07      // push 2byte (signed) value.
#define SIZEOF_PUSH2 3
#define PUSH1 0X08      // push 1byte (signed) value.
#define SIZEOF_PUSH1 2
/* ARITHMETIC OPERATORS */
// pops b, then pops a, does a 'op b and pushes result on top.
#define ADD 0X09
#define SIZEOF_ADD 1
#define SUB 0X0A
#define SIZEOF_SUB 1
#define MUL 0X0B
#define SIZEOF_MUL 1
#define DIV 0X0C
#define SIZEOF_DIV 1
#define MOD 0X0D
#define SIZEOF_MOD 1
/* COMPARISONS */
// pops b, then pops a, does a 'op b and pushes 1 if true, else 0.
#define EQ 0X0E
#define SIZEOF_EQ 1
#define NE 0X0F
#define SIZEOF_NE 1
#define LT 0X10
#define SIZEOF_LT 1
#define GT 0X11
#define SIZEOF_GT 1
#define LE 0X12
#define SIZEOF_LE 1
#define GE 0X13
#define SIZEOF_GE 1
/* LOGICAL OPERATORS */
#define NOT 0X14
#define SIZEOF_NOT 1
#define AND 0X15
#define SIZEOF_AND 1
#define OR 0X16
#define SIZEOF_OR 1
/* IO OPERATORS */
#define INPUT 0X17      // read character from stdin, pushes ASCII value.
#define SIZEOF_INPUT 1
#define OUTPUT 0X18     // pops ASCII value from stack, prints character to stdout.
#define SIZEOF_OUTPUT 1
#define CLOCK 0X2A
#define SIZEOF_CLOCK 1  // prints the elapsed time since start of execution.
#define CONS 0X30
#define SIZEOF_CONS 1   // pops b, then pops a, allocates cons(a, b) on the heap
                        // and pushes the address on the stack.
#define HD 0X31         // pops a cons address and pushes its head.
#define SIZEOF_HD 1
#define TL 0X32         // pops a cons adderss and pushed its tail.
#define SIZEOF_TL 1

