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
#define JNZ 0X02        // pop, and jump to address (2 bytes) if popped element not zero.
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
/* ARITHMETIC OPERATORS */ // pops b, then pops a, does a 'op b and pushes result on top.
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
/* COMPARISONS */       // pops b, then pops a, does a 'op b and pushes 1 if true, else 0.
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
#define INPUT 0X17      // read char from stdin, pushes ASCII value.
#define SIZEOF_INPUT 1
#define OUTPUT 0X18     // pops ASCII value from stack, prints char to stdout.
#define SIZEOF_OUTPUT 1
#define CLOCK 0X2A
#define SIZEOF_CLOCK 1
/* TO BE IMPLEMENTED */
#define CONS 0X30
#define SIZEOF_CONS 1
#define HD 0X31
#define SIZEOF_HD 1
#define TL 0X32
#define SIZEOF_TL 1

char get1Byte(void *ptr)
{
    // signed
    char *address = ptr;
    char result = *address;
    return result;
}
int32_t get2Byte(void *ptr)
{
    // signed
    char *address = ptr;
    unsigned char first_byte = *address;
    char secnd_byte = *(address + 1);
    int32_t result = 0;
    // memcpy(&result, address, 2);
    result = (secnd_byte << 8) + first_byte;
    return result;
}
int32_t get4Byte(void *ptr)
{
    // signed
    char *address = ptr;
    unsigned char first_byte = *address;
    unsigned char secnd_byte = *(address+1);
    unsigned char third_byte = *(address+2);
    char forth_byte = *(address+3);
    int32_t result = 0;
    result = (forth_byte << 24) + (third_byte << 16) + (secnd_byte << 8) + first_byte;
    return result;
}
int32_t get2ByteAddress(void *ptr)
{
    char *address = ptr;
    char first_byte = *address;
    unsigned char secnd_byte = *(address + 1);
    uint32_t result = 0;
    // memcpy(&result, address, 2);
    result = (secnd_byte << 8) + first_byte;
    return result;
}

stack_t STACK_MACHINE;
char byte_program[MAX_PROGRAM];

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

    char opcode;
    char *pc = &byte_program[0];

    char char_input = 0, char_output = 0;
    intptr_t arg1 = 0,  arg2 = 0;
    cons *poppedCell = NULL;

    clock_t begin = clock();
    clock_t end = clock();
    double time_spent = 0.0;
    while(1)
    {
        opcode = pc[0];
        stackPrint(&STACK_MACHINE);
        switch (opcode)
        {
            case JUMP:
                arg1 = get2ByteAddress(&pc[1]);
                pc = &byte_program[arg1];
                break;
            case JNZ:
                arg1 = stackPop(&STACK_MACHINE);
                //stackPrint(&STACK_MACHINE);
                if (arg1 != 0)
                {
                    arg1 = get2ByteAddress(&pc[1]);
                    pc = &byte_program[arg1];
                }
                else
                    pc += SIZEOF_JNZ;
                break;
            case DUP:
                arg1 = (unsigned char) get1Byte(&pc[1]);
                stackDupPush(&STACK_MACHINE, arg1);
                pc += SIZEOF_DUP;
                break;
            case SWAP:
                arg1 = (unsigned char) get1Byte(&pc[1]);
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
            case ADD:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, arg1 + arg2);
                pc += SIZEOF_ADD;
                break;
            case SUB:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, arg1 - arg2);
                pc += SIZEOF_SUB;
                break;
            case MUL:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, arg1 * arg2);
                pc += SIZEOF_MUL;
                break;
            case DIV:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, arg1 / arg2); // int division
                pc += SIZEOF_DIV;
                break;
            case MOD:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
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
                stackPush(&STACK_MACHINE, (arg1 < arg2));
                pc += SIZEOF_LT;
                break;
            case GT:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 > arg2));
                pc += SIZEOF_GT;
                break;
            case LE:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
                stackPush(&STACK_MACHINE, (arg1 <= arg2));
                pc += SIZEOF_LE;
                break;
            case GE:
                arg2 = stackPop(&STACK_MACHINE);
                arg1 = stackPop(&STACK_MACHINE);
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
                // assume that the push order is tail - head .
                // push 0 3 cons -> cons {3, null}
                // push 0 3 cons push 2 cons -> cons {3, cons {2, null}} 
                // push 0 3 cons push 2 cons push 1 -> cons {3, cons {2, cons {1, null}}} 
                arg2 = stackPop(&STACK_MACHINE); // head
                arg1 = stackPop(&STACK_MACHINE); // tail
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
                return 0;
                break;
            default:
                printf("either end of stream or wrong opcode\n");
                return 0;
                break;
        }
    } 
}
