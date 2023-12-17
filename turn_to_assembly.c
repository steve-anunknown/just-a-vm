#include <stdio.h>
#include <stdlib.h>

#include "instructions.h"
#include "utils.h"

#define MAX_PROGRAM 65536
uint8_t program[MAX_PROGRAM];

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    FILE* byte_file = fopen(argv[1], "r");
    if (byte_file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        exit(1);
    }
    fread(program, sizeof(uint8_t), MAX_PROGRAM, byte_file);
    fclose(byte_file);
    // create output file
    FILE* assembly_file = fopen("assembly.s", "w");
    if (assembly_file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s\n", "assembly.s");
        exit(1);
    }

    uint8_t opcode;
    uint8_t* pc = &program[0];
    while(1)
    {
        opcode = pc[0];
        switch (opcode)
        {
            case JUMP:
                fprintf(assembly_file, "JUMP %lx\n", get2ByteAddress(&pc[1]));
                pc += SIZEOF_JUMP;
                break;
            case JNZ:
                fprintf(assembly_file, "JNZ %lx\n", get2ByteAddress(&pc[1]));
                pc += SIZEOF_JNZ;
                break;
            case DUP:
                fprintf(assembly_file, "DUP %lx\n", get1Byte(&pc[1]));
                pc += SIZEOF_DUP;
                break;
            case SWAP:
                fprintf(assembly_file, "SWAP %lx\n", get1Byte(&pc[1]));
                pc += SIZEOF_SWAP;
                break;
            case DROP:
                fprintf(assembly_file, "DROP\n");
                pc += SIZEOF_DROP;
                break;
                /* ==================IO OPERATORS===================== */
            case INPUT:
                fprintf(assembly_file, "INPUT\n");
                pc += SIZEOF_INPUT;
                break;
            case OUTPUT:
                fprintf(assembly_file, "OUTPUT\n");
                pc += SIZEOF_OUTPUT;
                break;
                /* ==================PUSH OPERATORS===================== */
            case PUSH1:
                fprintf(assembly_file, "PUSH1 %lx\n", get1Byte(&pc[1]));
                pc += SIZEOF_PUSH1;
                break;
            case PUSH2:
                fprintf(assembly_file, "PUSH2 %lx\n", get2Byte(&pc[1]));
                pc += SIZEOF_PUSH2;
                break;
            case PUSH4:
                fprintf(assembly_file, "PUSH4 %lx\n", get4Byte(&pc[1]));
                pc += SIZEOF_PUSH4;
                break;
            case ADD:
                fprintf(assembly_file, "ADD\n");
                pc += SIZEOF_ADD;
                break;
            case SUB:
                fprintf(assembly_file, "SUB\n");
                pc += SIZEOF_SUB;
                break;
            case MUL:
                fprintf(assembly_file, "MUL\n");
                pc += SIZEOF_MUL;
                break;
            case DIV:
                fprintf(assembly_file, "DIV\n");
                pc += SIZEOF_DIV;
                break;
            case MOD:
                fprintf(assembly_file, "MOD\n");
                pc += SIZEOF_MOD;
                break;
                /* =========================COMPARISONS======================= */
            case EQ:
                fprintf(assembly_file, "EQ\n");
                pc += SIZEOF_EQ;
                break;
            case NE:
                fprintf(assembly_file, "NE\n");
                pc += SIZEOF_NE;
                break;
            case LT:
                fprintf(assembly_file, "LT\n");
                pc += SIZEOF_LT;
                break;
            case GT:
                fprintf(assembly_file, "GT\n");
                pc += SIZEOF_GT;
                break;
            case LE:
                fprintf(assembly_file, "LE\n");
                pc += SIZEOF_LE;
                break;
            case GE:
                fprintf(assembly_file, "GE\n");
                pc += SIZEOF_GE;
                break;
                /* ======================LOGICAL OPERATORS==================== */
            case NOT:
                fprintf(assembly_file, "NOT\n");
                pc += SIZEOF_NOT;
                break;
            case AND:
                fprintf(assembly_file, "AND\n");
                pc += SIZEOF_AND;
                break;
            case OR:
                fprintf(assembly_file, "OR\n");
                pc += SIZEOF_OR;
                break;
                /* ======================DYNAMIC MEMORY======================= */
            case CONS:
                fprintf(assembly_file, "CONS\n");
                pc += SIZEOF_CONS;
                break;
            case HD:
                fprintf(assembly_file, "HD\n");
                pc += SIZEOF_HD;
                break;
            case TL:
                fprintf(assembly_file, "TL\n");
                pc += SIZEOF_TL;
                break;
                /* ===========================FINISH========================== */
            case CLOCK:
                fprintf(assembly_file, "CLOCK\n");
                pc += SIZEOF_CLOCK;
                break;
            case HALT:
                fprintf(assembly_file, "HALT\n");
                return 0;
                break;
            default:
                printf("either end of stream or wrong opcode\n");
                return 0;
                break;
        }
    }
}