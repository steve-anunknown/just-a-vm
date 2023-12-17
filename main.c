#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <err.h>

#include "instructions.h"
#include "utils.h"
#include "stack.h"
#include "cons.h"
#include "gc.h"
#include "bitarray.h"

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
    const size_t    PAGE_SIZE        = 4096;
    const uintptr_t MAX_HEAP_ADDRESS = 0x3FFFFFFFFFFFFFFF;
    uintptr_t       MIN_HEAP_ADDRESS = MAX_HEAP_ADDRESS - 4*PAGE_SIZE; 
    unsigned int    cons_counter = 0;
    GC.heap     = mmap((void*)MIN_HEAP_ADDRESS, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    GC.size     = PAGE_SIZE;
    GC.bitarray = calloc((PAGE_SIZE+1)/sizeof(uint32_t), sizeof(uint32_t));
    GC.bottom   = (uintptr_t) GC.heap;
    printf("heap address is %p.\n", (void*)GC.heap);

    while(1)
    {
        opcode = pc[0];
        //stackPrint(GC.machine);
        switch (opcode)
        {
            case JUMP:
                arg1 = get2ByteAddress(&pc[1]);
                pc = &byte_program[arg1];
                break;
            case JNZ:
                arg1 = stackPop(GC.machine);
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
                stackDupPush(GC.machine, arg1);
                pc += SIZEOF_DUP;
                break;
            case SWAP:
                arg1 = get1Byte(&pc[1]);
                stackSwap(GC.machine, arg1);
                pc += SIZEOF_SWAP;
                break;
            case DROP:
                // pop and ignore
                stackPop(GC.machine);
                pc += SIZEOF_DROP;
                break;
                /* ==================IO OPERATORS===================== */
            case INPUT:
                char_input = getchar();
                stackPush(GC.machine, char_input);
                pc += SIZEOF_INPUT;
                break;
            case OUTPUT:
                char_output = stackPop(GC.machine);
                putchar(char_output);
                pc += SIZEOF_OUTPUT;
                break;
                /* ==================PUSH OPERATORS===================== */
            case PUSH1:
                arg1 = get1Byte(&pc[1]);
                stackPush(GC.machine, arg1 & GC_MASK);
                pc += SIZEOF_PUSH1;
                break;
            case PUSH2:
                arg1 = get2Byte(&pc[1]);
                stackPush(GC.machine, arg1 & GC_MASK);
                pc += SIZEOF_PUSH2;
                break;
            case PUSH4:
                arg1 = get4Byte(&pc[1]);
                stackPush(GC.machine, arg1 & GC_MASK);
                pc += SIZEOF_PUSH4;
                break;
                /* ==================ARITHMETIC OPERATORS===================== */
                /*
                 * operators are 31 or 63 bit.
                 * (depending on the machine.)
                 * 1 bit has to be retained for
                 * garbage collection purposes.
                 */
            case ADD:
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 + arg2) & GC_MASK;
                stackPush(GC.machine, result);
                pc += SIZEOF_ADD;
                break;
            case SUB:
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 - arg2) & GC_MASK;
                stackPush(GC.machine, result);
                pc += SIZEOF_SUB;
                break;
            case MUL:
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 * arg2) & GC_MASK;
                stackPush(GC.machine, result);
                pc += SIZEOF_MUL;
                break;
            case DIV:
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 / arg2) & GC_MASK;
                stackPush(GC.machine, result); 
                pc += SIZEOF_DIV;
                break;
            case MOD:
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 % arg2) & GC_MASK;
                stackPush(GC.machine, result);
                pc += SIZEOF_MOD;
                break;
                /* =========================COMPARISONS======================= */
                /*
                 * the bytes that have been pushed on the stack are signed.
                 * therefore, eventhough an unsigned type is used to represent
                 * the data on the stack, the comparison operators must operate
                 * on signed types. Therefore, the data are casted to signed
                 * types before the comparison.
                */
            case EQ:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 == arg2));
                pc += SIZEOF_EQ;
                break;
            case NE:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != arg2));
                pc += SIZEOF_NE;
                break;
            case LT:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 < (intptr_t)arg2));
                pc += SIZEOF_LT;
                break;
            case GT:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 > (intptr_t)arg2));
                pc += SIZEOF_GT;
                break;
            case LE:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 <= (intptr_t)arg2));
                pc += SIZEOF_LE;
                break;
            case GE:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 >= (intptr_t)arg2));
                pc += SIZEOF_GE;
                break;
                /* ======================LOGICAL OPERATORS==================== */
            case NOT:
                arg1 = stackPop((GC.machine));
                stackPush(GC.machine, (arg1 != 0));
                pc += SIZEOF_NOT;
                break;
            case AND:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != 0 && arg2 != 0));
                pc += SIZEOF_AND;
                break;
            case OR:
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != 0 || arg2 != 0));
                pc += SIZEOF_OR;
                break;
                /* ======================DYNAMIC MEMORY======================= */
            case CONS:
                cons_counter++;
                if (!GC.freelist)
                {
                    /*
                     * This returns true if GC.freelist turns out not empty.
                     * This value can then be checked and perhaps more space on
                     * the heap can be allocated.
                     */
                    printf("Garbage collector activated: cons counter = %d\n", cons_counter);
                    cons_counter = 0;
                    stackPrint(GC.machine);
                    if (markAndSweep(&GC))
                        printf("freelist address is %p.\nMemory has been freed, all good.\n\n\n", (void*)GC.freelist);
                    else
                    {
                        // TODO: try to allocate more space on the heap, if it runs out.
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
                arg2             = stackPop(GC.machine); // tail
                poppedCell->tail = (cons*) arg2; 

                /*
                 * This must also NOT be masked, irregardless of what it is, for
                 * the same reason as above.
                 */ 
                arg1             = stackPop(GC.machine); // head
                poppedCell->head = arg1;

                stackPush(GC.machine, ((uintptr_t) poppedCell) | MARK_FAKE);
                pc += SIZEOF_CONS;
                break;
            case HD:
                poppedCell = (cons *) (stackPop(GC.machine) & GC_MASK);

                stackPush(GC.machine, poppedCell->head);
                pc += SIZEOF_HD;
                break;
            case TL:
                poppedCell = (cons *) (stackPop(GC.machine) & GC_MASK);

                stackPush(GC.machine, (uintptr_t) poppedCell->tail);
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