#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

#define MAX_PROGRAM 65536

#define HALT 0x00

#define JUMP 0x01       // jump to address (2 bytes).
#define SIZEOF_JUMP 3
#define JNZ 0x02        // pop, and jump to address (2 bytes) if popped element not zero.
#define SIZEOF_JNZ 3
#define DUP 0x03        // push i-th (1 unsigned byte) element to the top of the stack.
#define SIZEOF_DUP 2
#define SWAP 0x04       // swap top of the stack and i-th (1 unsigned byte) element.
#define SIZEOF_SWAP 2
#define DROP 0x05       // pop and ignore.
#define SIZEOF_DROP 2
/* PUSH OPERATORS */
#define PUSH4 0x06      // push 4byte (signed) value.
#define SIZEOF_PUSH4 5
#define PUSH2 0x07      // push 2byte (signed) value.
#define SIZEOF_PUSH2 3
#define PUSH1 0x08      // push 1byte (signed) value.
#define SIZEOF_PUSH1 2
/* ARITHMETIC OPERATORS */ // pops b, then pops a, does a 'op b and pushes result on top.
#define ADD 0x09
#define SIZEOF_ADD 1
#define SUB 0x0A
#define SIZEOF_SUB 1
#define MUL 0x0B
#define SIZEOF_MUL 1
#define DIV 0x0C
#define SIZEOF_DIV 1
#define MOD 0x0D
#define SIZEOF_MOD 1
/* COMPARISONS */       // pops b, then pops a, does a 'op b and pushes 1 if true, else 0.
#define EQ 0x0E
#define SIZEOF_EQ 1
#define NE 0x0F
#define SIZEOF_NE 1
#define LT 0x10
#define SIZEOF_LT 1
#define GT 0x11
#define SIZEOF_GT 1
#define LE 0x12
#define SIZEOF_LE 1
#define GE 0x13
#define SIZEOF_GE 1
/* LOGICAL OPERATORS */
#define NOT 0X14
#define SIZEOF_NOT 1
#define AND 0X15
#define SIZEOF_AND 1
#define OR 0X16
#define SIZEOF_OR 1
/* IO OPERATORS */
#define INPUT 0x17      // read char from stdin, pushes ASCII value.
#define SIZEOF_INPUT 1
#define OUTPUT 0x18     // pops ASCII value from stack, prints char to stdout.
#define SIZEOF_OUTPUT 1

char get1Byte(void *ptr)
{
    // signed
    char *address = ptr;
    char result = *address;
    return result;
}
int get2Byte(void *ptr)
{
    // signed
    char *address = ptr;
    char first_byte = *address;
    char secnd_byte = *(address + 1);
    int result = 0;
    // memcpy(&result, address, 2);
    result = (first_byte << 8) + secnd_byte;
    return result;
}
int get4Byte(void *ptr)
{
    // signed
    char *address = ptr;
    char first_byte = *address;
    char secnd_byte = *(address+1);
    char third_byte = *(address+2);
    char forth_byte = *(address+3);
    int result = 0;
    result = (first_byte << 24) + (secnd_byte << 16) + (third_byte << 8) + forth_byte;
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
    int byte_count = fread(byte_program, sizeof(char), MAX_PROGRAM, byte_file);
    char opcode;
    char *pc = &byte_program[0];
    char char_input = 0, char_output = 0;
    int arg1 = 0,  arg2 = 0;
    while(1)
    {
        stackPrint(&STACK_MACHINE);
        opcode = pc[0];
        switch (opcode)
        {
            case JUMP:
                arg1 = get2Byte(&pc[1]);
                pc = &byte_program[arg1];
                break;
            case JNZ:
                arg1 = stackPop(&STACK_MACHINE);
                pc = (arg1 == 0) ? &byte_program[get2Byte(&pc[1])] : pc + SIZEOF_JNZ;
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
            /* ===========================FINISH========================== */
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
