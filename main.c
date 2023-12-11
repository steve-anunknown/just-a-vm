#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "stack.h"
#include "cons.h"

#define MAX_PROGRAM 65536

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

#define GC_MASK      (0x3FFFFFFFFFFFFFF)
#define GC_MASK_COMP (0xC00000000000000)
#define INTEGER_SIG  (0x800000000000000)
#define SHIFT_1_BYTE 8
#define SHIFT_2_BYTE 16
#define SHIFT_3_BYTE 24

/*
 * All of the below getByte functions return
 * something that will be pushed on the stack.
 * Therefore, it has to be a uintptr_t thing.
 */

uintptr_t get1Byte(void *ptr)
{
    uint8_t *address = ptr;
    uintptr_t result = 0;
    result = (*address) & (0x00000000000000FF);
    if ((*address) & 0x80)
        // sign extend it.
        result = result | (0x3FFFFFFFFFFFFF00); 
    return result;
}
uintptr_t get2Byte(void *ptr)
{
    uint8_t *address = ptr;
    uint8_t first_byte = *address;
    uint8_t secnd_byte = *(address + 1);
    uintptr_t result = 0;
    result = ((secnd_byte << SHIFT_1_BYTE) + first_byte);
    if (secnd_byte & 0x80)
        // sign extend it.
        result = result | (0x3FFFFFFFFFFF0000);
    return result;
}
uintptr_t get2ByteAddress(void *ptr)
{
    uint8_t *address = ptr;
    uint8_t first_byte = *address;
    uint8_t secnd_byte = *(address + 1);
    uintptr_t result = 0;
    result = ((secnd_byte << SHIFT_1_BYTE) + first_byte);
    return result;
}
uintptr_t get4Byte(void *ptr)
{
    uint8_t *address = ptr;
    uint8_t first_byte = *address;
    uint8_t secnd_byte = *(address+1);
    uint8_t third_byte = *(address+2);
    uint8_t forth_byte = *(address+3);
    uintptr_t result = 0;
    result = ((forth_byte << SHIFT_3_BYTE) +
              (third_byte << SHIFT_2_BYTE) + 
              (secnd_byte << SHIFT_1_BYTE) +
              first_byte);
    if (forth_byte & 0x80)
        // sign extend it.
        result = result | (0x3FFFFFFF00000000);
    result = result & GC_MASK;
    return result;
}

int32_t intCheck(uintptr_t lhs, uintptr_t rhs)
{
    // unused for now.
    uintptr_t lbits = (lhs & GC_MASK_COMP) << 1;
    uintptr_t rbits = (rhs & GC_MASK_COMP) << 1;
    return (lbits == rbits && lbits == INTEGER_SIG);
}

stack_t STACK_MACHINE;
uint8_t byte_program[MAX_PROGRAM];

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./vm <bytecodefile>\n");
        exit(1);
    }
    FILE *byte_file = fopen(argv[1], "r");
    if (!byte_file)
    {
        perror("error opening file");
        exit(1);
    }
    int32_t byte_count = fread(byte_program, sizeof(char), MAX_PROGRAM, byte_file);
    if (byte_count == 0)
    {
        fprintf(stderr, "either fread failed or file is empty\n");
        exit(1);
    }

    uint8_t opcode;
    uint8_t *pc = &byte_program[0];

    uint8_t char_input = 0, char_output = 0;
    uintptr_t arg1 = 0,  arg2 = 0, result = 0;
    cons *poppedCell = NULL;

    clock_t begin = clock();
    clock_t end   = clock();
    double time_spent = 0.0;
    while(1)
    {
        opcode = pc[0];
        switch (opcode)
        {
            case JUMP:
                arg1 = get2ByteAddress(&pc[1]);
                pc = &byte_program[arg1];
                break;
            case JNZ:
                arg1 = stackPop(&STACK_MACHINE);
                if (arg1 != 0)
                {
                    arg1 = get2ByteAddress(&pc[1]);
                    pc = &byte_program[arg1];
                }
                else
                    pc += SIZEOF_JNZ;
                break;
            case DUP:
                arg1 = get1Byte(&pc[1]);
                stackDupPush(&STACK_MACHINE, arg1);
                pc += SIZEOF_DUP;
                break;
            case SWAP:
                arg1 = get1Byte(&pc[1]);
                stackSwap(&STACK_MACHINE, arg1);
                pc += SIZEOF_SWAP;
                break;
            case DROP:
                // pop and ignore
                stackPop(&STACK_MACHINE);
                pc += SIZEOF_DROP;
                break;
            /* ==================IO OPERATORS===================== */
            case INPUT:
                char_input = getchar();
                stackPush(&STACK_MACHINE, char_input);
                pc += SIZEOF_INPUT;
                break;
            case OUTPUT:
                char_output = stackPop(&STACK_MACHINE);
                putchar(char_output);
                pc += SIZEOF_OUTPUT;
                break;
            /* ==================PUSH OPERATORS===================== */
            case PUSH1:
                arg1 = get1Byte(&pc[1]);
                stackPush(&STACK_MACHINE, arg1);
                pc += SIZEOF_PUSH1;
                break;
            case PUSH2:
                arg1 = get2Byte(&pc[1]);
                stackPush(&STACK_MACHINE, arg1);
                pc += SIZEOF_PUSH2;
                break;
            case PUSH4:
                arg1 = get4Byte(&pc[1]);
                stackPush(&STACK_MACHINE, arg1);
                pc += SIZEOF_PUSH4;
                break;
            /* ==================ARITHMETIC OPERATORS===================== */
            /*
             * operators are 30 or 62 bit.
             * (depending on the machine.)
             * 2 bits have to be retained for
             * garbage collection purposes.
             */
            case ADD:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                result = (arg1 + arg2) & (GC_MASK);
                stackPush(&STACK_MACHINE, result);
                pc += SIZEOF_ADD;
                break;
            case SUB:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                result = (arg1 - arg2) & (GC_MASK);
                stackPush(&STACK_MACHINE, result);
                pc += SIZEOF_SUB;
                break;
            case MUL:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                result = (arg1 * arg2) & (GC_MASK);
                stackPush(&STACK_MACHINE, result);
                pc += SIZEOF_MUL;
                break;
            case DIV:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                result = (arg1 / arg2) & (GC_MASK);
                stackPush(&STACK_MACHINE, result); 
                pc += SIZEOF_DIV;
                break;
            case MOD:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                result = (arg1 % arg2) & (GC_MASK);
                stackPush(&STACK_MACHINE, arg1 % arg2);
                pc += SIZEOF_MOD;
                break;
            /* =========================COMPARISONS======================= */
            case EQ:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 == arg2));
                pc += SIZEOF_EQ;
                break;
            case NE:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 != arg2));
                pc += SIZEOF_NE;
                break;
            case LT:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                arg2 = arg2 << 2; // discard the 2 gc bits.
                arg1 = arg1 << 2; // this pads zeros so it's ok.
                stackPush(&STACK_MACHINE, (arg1 < arg2));
                pc += SIZEOF_LT;
                break;
            case GT:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                arg2 = arg2 << 2; // discard the 2 gc bits.
                arg1 = arg1 << 2; // this pads zeros so it's ok.
                stackPush(&STACK_MACHINE, (arg1 > arg2));
                pc += SIZEOF_GT;
                break;
            case LE:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                arg2 = arg2 << 2; // discard the 2 gc bits.
                arg1 = arg1 << 2; // this pads zeros so it's ok.
                stackPush(&STACK_MACHINE, (arg1 <= arg2));
                pc += SIZEOF_LE;
                break;
            case GE:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                arg2 = arg2 << 2; // discard the 2 gc bits.
                arg1 = arg1 << 2; // this pads zeros so it's ok.
                stackPush(&STACK_MACHINE, (arg1 >= arg2));
                pc += SIZEOF_GE;
                break;
            /* ======================LOGICAL OPERATORS==================== */
            case NOT:
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 != 0));
                pc += SIZEOF_NOT;
                break;
            case AND:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 != 0 && arg2 != 0));
                pc += SIZEOF_AND;
                break;
            case OR:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 != 0 || arg2 != 0));
                pc += SIZEOF_AND;
                break;
            /* ======================DYNAMIC MEMORY======================= */
            case CONS:
                /*
                 * assume that the push order is tail - head .
                 * (I don't even think there is a different valid order.)
                 * push 0 3 cons -> cons {3, null}
                 * push 0 3 cons push 2 cons -> cons {3, cons {2, null}} 
                 * push 0 3 cons push 2 cons push 1 -> cons {3, cons {2, cons {1, null}}} 
                 */
                arg2 = stackPop(&STACK_MACHINE); // head
                arg1 = stackPop(&STACK_MACHINE); // tail
                /*
                 * this is enough for simple dynamic memory allocation,
                 * but not enough for a garbace collection implementation.
                 * I think that I have to allocate some space at the
                 * beginning, manage this space with a custom malloc,
                 * and increase it if need be.
                 */
                cons *newCell = malloc(sizeof(cons));
                newCell->tail = (cons *) arg1;
                newCell->head = arg2;
                stackPush(&STACK_MACHINE, (intptr_t) newCell);
                pc += SIZEOF_CONS;
                break;
            case HD:
                // hopefully a cons-cell address is popped
                poppedCell = (cons *) stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, poppedCell->head);
                pc += SIZEOF_HD;
                break;
            case TL:
                // hopefully a cons-cell address is popped
                poppedCell = (cons *) stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (intptr_t) poppedCell->tail);
                pc += SIZEOF_TL;
                break;
            /* ===========================FINISH========================== */
            case CLOCK:
                end = clock();
                time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("%0.6lf\n", time_spent);
                pc += SIZEOF_CLOCK;
                break;
            case HALT:
                printf("Halting.\n");
                return 0;
                break;
            default:
                printf("either end of stream or wrong opcode\n");
                return 0;
                break;
        }
    } 
}
