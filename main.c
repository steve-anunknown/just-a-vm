#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <err.h>

#include "instructions.h"   // this includes the opcodes, labels and sizes
#include "utils.h"          // this includes the getByte functions
#include "stack.h"          // this includes the stack functions and stack definition
#include "cons.h"           // this includes the cons cell definition
#include "gc.h"             // this includes the garbage collector functions and definition
#include "bitarray.h"       // this includes the bitarray functions and definition

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

    while(1)
    {
    next_instruction:
        opcode = pc[0];
        switch (opcode)
        {
            case JUMP:
                arg1 = get2ByteAddress(&pc[1]);
                pc = &byte_program[arg1];
                goto next_instruction;
            case JNZ:
                arg1 = stackPop(GC.machine);
                if (arg1 != 0)
                {
                    arg1 = get2ByteAddress(&pc[1]);
                    pc = &byte_program[arg1];
                }
                else
                    pc += SIZEOF_JNZ;
                goto next_instruction;
            case DUP:
                arg1 = get1Byte(&pc[1]);
                stackDupPush(GC.machine, arg1);
                pc += SIZEOF_DUP;
                goto next_instruction;
            case SWAP:
                arg1 = get1Byte(&pc[1]);
                pc += SIZEOF_SWAP;
                stackSwap(GC.machine, arg1);
                goto next_instruction;
            case DROP:
                // pop and ignore
                pc += SIZEOF_DROP;
                stackPop(GC.machine);
                goto next_instruction;
                /* ==================PUSH OPERATORS===================== */
            case PUSH1:
                arg1 = get1Byte(&pc[1]);
                pc += SIZEOF_PUSH1;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto next_instruction;
            case PUSH2:
                arg1 = get2Byte(&pc[1]);
                pc += SIZEOF_PUSH2;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto next_instruction;
            case PUSH4:
                arg1 = get4Byte(&pc[1]);
                pc += SIZEOF_PUSH4;
                stackPush(GC.machine, arg1 & GC_MASK);
                goto next_instruction;
                /* ==================ARITHMETIC OPERATORS===================== */
                /*
                 * operators are 31 or 63 bit.
                 * (depending on the machine.)
                 * 1 bit has to be retained for
                 * garbage collection purposes.
                 */
            case ADD:
          
                pc += SIZEOF_ADD;
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 + arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto next_instruction;
            case SUB:
                pc += SIZEOF_SUB;
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 - arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto next_instruction;
            case MUL:
                pc += SIZEOF_MUL;
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 * arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto next_instruction;
            case DIV:
                pc += SIZEOF_DIV;
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 / arg2) & GC_MASK;
                stackPush(GC.machine, result); 
                goto next_instruction;
            case MOD:
                pc += SIZEOF_MOD;
                arg2   = stackPop(GC.machine);
                arg1   = stackPop(GC.machine);
                result = (arg1 % arg2) & GC_MASK;
                stackPush(GC.machine, result);
                goto next_instruction;
                /* =========================COMPARISONS======================= */
                /*
                 * the bytes that have been pushed on the stack are signed.
                 * therefore, eventhough an unsigned type is used to represent
                 * the data on the stack, the comparison operators must operate
                 * on signed types. Therefore, the data are casted to signed
                 * types before the comparison.
                */
            case EQ:
                pc += SIZEOF_EQ;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 == arg2));
                goto next_instruction;
            case NE:
                pc += SIZEOF_NE;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != arg2));
                goto next_instruction;
            case LT:
                pc += SIZEOF_LT;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 < (intptr_t)arg2));
                goto next_instruction;
            case GT:
                pc += SIZEOF_GT;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 > (intptr_t)arg2));
                goto next_instruction;
            case LE:
                pc += SIZEOF_LE;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 <= (intptr_t)arg2));
                goto next_instruction;
            case GE:
                pc += SIZEOF_GE;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                arg2 = arg2 << 1; // discard the 1 gc bit.
                arg1 = arg1 << 1; // this pads zeros so it's ok.
                stackPush(GC.machine, ((intptr_t)arg1 >= (intptr_t)arg2));
                goto next_instruction;
                /* ======================LOGICAL OPERATORS==================== */
            case NOT:
                pc += SIZEOF_NOT;
                arg1 = stackPop((GC.machine));
                stackPush(GC.machine, (arg1 != 0));
                goto next_instruction;
            case AND:
                pc += SIZEOF_AND;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != 0 && arg2 != 0));
                goto next_instruction;
            case OR:
                pc += SIZEOF_OR;
                arg2 = stackPop(GC.machine);
                arg1 = stackPop(GC.machine);
                stackPush(GC.machine, (arg1 != 0 || arg2 != 0));
                goto next_instruction;
                /* ==================IO OPERATORS===================== */
            case INPUT:
                pc += SIZEOF_INPUT;
                char_input = getchar();
                stackPush(GC.machine, char_input);
                goto next_instruction;
            case OUTPUT:
                pc += SIZEOF_OUTPUT;
                char_output = stackPop(GC.machine);
                putchar(char_output);
                goto next_instruction;
                /* ======================DYNAMIC MEMORY======================= */
            case CONS:
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
                arg2             = stackPop(GC.machine); // tail
                poppedCell->tail = (cons*) arg2; 

                /*
                 * This must also NOT be masked, irregardless of what it is, for
                 * the same reason as above.
                 */ 
                arg1             = stackPop(GC.machine); // head
                poppedCell->head = arg1;

                stackPush(GC.machine, ((uintptr_t) poppedCell) | MARK_FAKE);
                goto next_instruction;
            case HD:
                pc += SIZEOF_HD;
                poppedCell = (cons *) (stackPop(GC.machine) & GC_MASK);

                stackPush(GC.machine, poppedCell->head);
                goto next_instruction;
            case TL:
                pc += SIZEOF_TL;
                poppedCell = (cons *) (stackPop(GC.machine) & GC_MASK);
                stackPush(GC.machine, (uintptr_t) poppedCell->tail);
                
                goto next_instruction;
                /* ===========================FINISH========================== */
            case CLOCK:
                pc += SIZEOF_CLOCK;
                end = clock();
                time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("%0.6lf\n", time_spent);
                goto next_instruction;
            case HALT:
                printf("Halting.\n");
                return 0;
            default:
                printf("either end of stream or wrong opcode\n");
                return 0;
        }
    } 
}
