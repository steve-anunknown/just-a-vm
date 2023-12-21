#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <err.h>

#include "instructions.h" // this includes the opcodes, labels and sizes
#include "utils.h"        // this includes the getByte functions
#include "stack.h"        // this includes the stack functions and stack definition
#include "cons.h"         // this includes the cons cell definition
#include "gc.h"           // this includes the garbage collector functions and definition
#include "bitarray.h"     // this includes the bitarray functions and definition

#define MAX_PROGRAM      65536
uint8_t byte_program[MAX_PROGRAM];

stack_t STACK_MACHINE;
garbage_collector GC = {&STACK_MACHINE, NULL, 0, 0, 0, NULL, NULL};

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
    int32_t byte_count = fread(byte_program, sizeof(uint8_t), MAX_PROGRAM, byte_file);
    if (byte_count == 0)
    {
        fprintf(stderr, "either fread failed or file is empty\n");
        exit(1);
    }

    uint8_t   opcode;
    uint8_t   *pc = &byte_program[0];
    uint8_t   char_input  = 0, char_output = 0;
    uintptr_t arg1        = 0, arg2        = 0, result = 0;
    cons      *poppedCell = NULL;

    clock_t begin     = clock();
    clock_t end       = clock();
    double time_spent = 0.0;

    // every item on the heap is a cons cell
    const size_t    PAGE_SIZE        = 10*4096;
    const uintptr_t MAX_HEAP_ADDRESS = 0x3FFFFFFFFFFFFFFF;
    uintptr_t       MIN_HEAP_ADDRESS = MAX_HEAP_ADDRESS - 4*PAGE_SIZE;
    GC.heap     = mmap((void*)MIN_HEAP_ADDRESS, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    GC.size     = PAGE_SIZE;
    GC.bitarray = calloc((PAGE_SIZE+1)/sizeof(uint32_t), sizeof(uint32_t));
    GC.bottom   = (uintptr_t) GC.heap;

    static void* labels[] = { // the indices must match the opcodes
        /*index 0*/&&L_HALT,
        /*index 1*/&&L_JUMP, 
        /*index 2*/&&L_JNZ, 
        /*index 3*/&&L_DUP, 
        /*index 4*/&&L_SWAP, 
        /*index 5*/&&L_DROP,
        /*index 6*/&&L_PUSH4,
        /*index 7*/&&L_PUSH2,
        /*index 8*/&&L_PUSH1,
        /*index 9*/&&L_ADD,
        /*index 10*/&&L_SUB,
        /*index 11*/&&L_MUL,
        /*index 12*/&&L_DIV,
        /*index 13*/&&L_MOD,
        /*index 14*/&&L_EQ,
        /*index 15*/&&L_NE,
        /*index 16*/&&L_LT,
        /*index 17*/&&L_GT,
        /*index 18*/&&L_LE,
        /*index 19*/&&L_GE,
        /*index 20*/&&L_NOT,
        /*index 21*/&&L_AND,
        /*index 22*/&&L_OR,
        /*index 23*/&&L_INPUT,
        /*index 24*/&&L_OUTPUT,
        /*index 25*/&&L_DEFAULT,
        /*index 26*/&&L_DEFAULT,
        /*index 27*/&&L_DEFAULT,
        /*index 28*/&&L_DEFAULT,
        /*index 29*/&&L_DEFAULT,
        /*index 30*/&&L_DEFAULT,
        /*index 31*/&&L_DEFAULT,
        /*index 32*/&&L_DEFAULT,
        /*index 33*/&&L_DEFAULT,
        /*index 34*/&&L_DEFAULT,
        /*index 35*/&&L_DEFAULT,
        /*index 36*/&&L_DEFAULT,
        /*index 37*/&&L_DEFAULT,
        /*index 38*/&&L_DEFAULT,
        /*index 39*/&&L_DEFAULT,
        /*index 40*/&&L_DEFAULT,
        /*index 41*/&&L_DEFAULT,
        /*index 42*/&&L_CLOCK,
        /*index 43*/&&L_DEFAULT,
        /*index 44*/&&L_DEFAULT,
        /*index 45*/&&L_DEFAULT,
        /*index 46*/&&L_DEFAULT,
        /*index 47*/&&L_DEFAULT,
        /*index 48*/&&L_CONS,
        /*index 49*/&&L_HD,
        /*index 50*/&&L_TL       
    };

    while(1)
    {
        opcode = pc[0];
        switch (opcode)
        {
            case JUMP:
L_JUMP:
                arg1 = get2ByteAddress(&pc[1]);
                pc = &byte_program[arg1];
                goto *(void *)(labels[*pc]);
            case JNZ:
L_JNZ:
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                if (arg1 != 0)
                {
                    arg1 = get2ByteAddress(&pc[1]);
                    pc = &byte_program[arg1];
                }
                else
                    pc += SIZEOF_JNZ;
                goto *(void *)(labels[*pc]);
            case DUP:
L_DUP:
                arg1 = get1Byte(&pc[1]);
                stackDupPush(GC.machine, arg1);
                pc += SIZEOF_DUP;
                goto *(void *)(labels[*pc]);
            case SWAP:
L_SWAP:
                arg1 = get1Byte(&pc[1]);
                pc += SIZEOF_SWAP;
                stackSwap(GC.machine, arg1);
                goto *(void *)(labels[*pc]);
            case DROP:
L_DROP:
                // pop and ignore
                pc += SIZEOF_DROP;
                STACK_MACHINE.top--;
                goto *(void *)(labels[*pc]);
                /* ==================PUSH OPERATORS===================== */
            case PUSH1:
L_PUSH1:
                arg1 = get1Byte(&pc[1]);
                pc += SIZEOF_PUSH1;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto *(void *)(labels[*pc]);
            case PUSH2:
L_PUSH2:
                arg1 = get2Byte(&pc[1]);
                pc += SIZEOF_PUSH2;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto *(void *)(labels[*pc]);
            case PUSH4:
L_PUSH4:
                arg1 = get4Byte(&pc[1]);
                pc += SIZEOF_PUSH4;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto *(void *)(labels[*pc]);
                /* ==================ARITHMETIC OPERATORS===================== */
                /*
                 * operators are 31 or 63 bit.
                 * (depending on the machine.)
                 * 1 bit has to be retained for
                 * garbage collection purposes.
                 */
            case ADD:
L_ADD:
                pc += SIZEOF_ADD;
                arg2   = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1   = STACK_MACHINE.data[--STACK_MACHINE.top];
                result = (arg1 + arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto *(void *)(labels[*pc]);
            case SUB:
L_SUB:
                pc += SIZEOF_SUB;
                arg2   = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1   = STACK_MACHINE.data[--STACK_MACHINE.top];
                result = (arg1 - arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto *(void *)(labels[*pc]);
            case MUL:
L_MUL:
                pc += SIZEOF_MUL;
                arg2   = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1   = STACK_MACHINE.data[--STACK_MACHINE.top];
                result = (arg1 * arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto *(void *)(labels[*pc]);
            case DIV:
L_DIV:
                pc += SIZEOF_DIV;
                arg2   = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1   = STACK_MACHINE.data[--STACK_MACHINE.top];
                result = (arg1 / arg2) & GC_MASK;
                stackPush(GC.machine, result); 
                goto *(void *)(labels[*pc]);
            case MOD:
L_MOD:
                pc += SIZEOF_MOD;
                arg2   = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1   = STACK_MACHINE.data[--STACK_MACHINE.top];
                result = (arg1 % arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto *(void *)(labels[*pc]);
                /* =========================COMPARISONS======================= */
                /*
                 * the bytes that have been pushed on the stack are signed.
                 * therefore, eventhough an unsigned type is used to represent
                 * the data on the stack, the comparison operators must operate
                 * on signed types. Therefore, the data are casted to signed
                 * types before the comparison.
                */
            case EQ:
L_EQ:
                pc += SIZEOF_EQ;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                stackPush(GC.machine, (arg1 == arg2));
                goto *(void *)(labels[*pc]);
            case NE:
L_NE:
                pc += SIZEOF_NE;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                stackPush(GC.machine, (arg1 != arg2));
                goto *(void *)(labels[*pc]);
            case LT:
L_LT:
                pc += SIZEOF_LT;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 < (intptr_t)arg2));
                goto *(void *)(labels[*pc]);
            case GT:
L_GT:
                pc += SIZEOF_GT;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 > (intptr_t)arg2));
                goto *(void *)(labels[*pc]);
            case LE:
L_LE:
                pc += SIZEOF_LE;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 <= (intptr_t)arg2));
                goto *(void *)(labels[*pc]);
            case GE:
L_GE:
                pc += SIZEOF_GE;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 >= (intptr_t)arg2));
                goto *(void *)(labels[*pc]);
                /* ======================LOGICAL OPERATORS==================== */
            case NOT:
L_NOT:
                pc += SIZEOF_NOT;
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                stackPush(GC.machine, (arg1 != 0));
                goto *(void *)(labels[*pc]);
            case AND:
L_AND:
                pc += SIZEOF_AND;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                stackPush(GC.machine, (arg1 != 0 && arg2 != 0));
                goto *(void *)(labels[*pc]);
            case OR:
L_OR:
                pc += SIZEOF_OR;
                arg2 = STACK_MACHINE.data[--STACK_MACHINE.top];
                arg1 = STACK_MACHINE.data[--STACK_MACHINE.top];
                stackPush(GC.machine, (arg1 != 0 || arg2 != 0));
                goto *(void *)(labels[*pc]);
                /* ==================IO OPERATORS===================== */
            case INPUT:
L_INPUT:
                pc += SIZEOF_INPUT;
                char_input = getchar();
                stackPush(GC.machine, char_input);
                goto *(void *)(labels[*pc]);
            case OUTPUT:
L_OUTPUT:
                pc += SIZEOF_OUTPUT;
                char_output = STACK_MACHINE.data[--STACK_MACHINE.top];
                putchar(char_output);
                goto *(void *)(labels[*pc]);
                /* ======================DYNAMIC MEMORY======================= */
            case CONS:
L_CONS:
                pc += SIZEOF_CONS;
                if (!GC.freelist)
                {
                    /*
                     * This returns true if GC.freelist turns out not empty.
                     * This value can then be checked and perhaps more space on
                     * the heap can be allocated.
                     */
                    if (!markAndSweep(&GC))
                    {
                        printf("Memory has been exhausted, trying to allocate more...\n");
                        exit(1);
                    }
                }
                poppedCell       = GC.freelist;            // this is a real address
                GC.freelist      = (cons*) GC.freelist->head;

                /*
                 * This must NOT be masked. Check the mark and sweep function in
                 * gc.c for more information.
                 */
                arg2             = STACK_MACHINE.data[--STACK_MACHINE.top]; // tail
                poppedCell->tail = (cons*) arg2; 

                /*
                 * This must also NOT be masked, irregardless of what it is, for
                 * the same reason as above.
                 */ 
                arg1             = STACK_MACHINE.data[--STACK_MACHINE.top]; // head
                poppedCell->head = arg1;

                stackPush(GC.machine, ((uintptr_t) poppedCell) | MARK_FAKE);
                goto *(void *)(labels[*pc]);
            case HD:
L_HD:
                pc += SIZEOF_HD;
                poppedCell = (cons *) (STACK_MACHINE.data[--STACK_MACHINE.top] & GC_MASK);

                stackPush(GC.machine, poppedCell->head);
                goto *(void *)(labels[*pc]);
            case TL:
L_TL:
                pc += SIZEOF_TL;
                poppedCell = (cons *) (STACK_MACHINE.data[--STACK_MACHINE.top] & GC_MASK);
                stackPush(GC.machine, (uintptr_t) poppedCell->tail);
                
                goto *(void *)(labels[*pc]);
                /* ===========================FINISH========================== */
            case CLOCK:
L_CLOCK:
                pc += SIZEOF_CLOCK;
                end = clock();
                time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("%0.6lf\n", time_spent);
                goto *(void *)(labels[*pc]);
            case HALT:
L_HALT:
                printf("Halting.\n");
                return 0;
            default:
L_DEFAULT:
                printf("either end of stream or wrong opcode\n");
                return 0;
        }
    } 
}
