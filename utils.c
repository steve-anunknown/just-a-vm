#include "utils.h"

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
    if ((*address) & 0x0000000000000080)
        // sign extend it.
        result = result | (0x7FFFFFFFFFFFFF00); 
    return result;
}
uintptr_t get2Byte(void *ptr)
{
    uint8_t *address = ptr;
    uint8_t first_byte = *address;
    uint8_t secnd_byte = *(address + 1);
    uintptr_t result = 0;
    result = ((secnd_byte << SHIFT_1_BYTE) + first_byte);
    if (secnd_byte & 0x00000000000080)
        // sign extend it.
        result = result | (0x7FFFFFFFFFFF0000);
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
    if (forth_byte & 0x00000000000080)
        // sign extend it.
        result = result | (0x7FFFFFFF00000000);
    result = result & GC_MASK;
    return result;
}
