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
    result = *address;
    result = result & 0xFF;
    if (result & 0x80)
        result = result | 0xFFFFFFFFFFFFFF00;
    return result;
}
uintptr_t get2Byte(void *ptr)
{
    uint8_t *address = ptr;
    uint8_t first_byte = *address;
    uint8_t secnd_byte = *(address + 1);
    uintptr_t result = 0;
    result = ((secnd_byte << SHIFT_1_BYTE) | first_byte);
    if (secnd_byte & 0x00000000000080)
        // sign extend it.
        result = result | (0x7FFFFFFFFFFF0000);
    return result;
}
uintptr_t get2ByteAddress(void *ptr)
{
    uint8_t *address = ptr;
    uintptr_t result = (address[1] << SHIFT_1_BYTE) | address[0];
    return result;
}
uintptr_t get4Byte(void *ptr)
{
    uint8_t *address = ptr;
    uintptr_t result = 0;
    result |= address[0];
    result |= (uintptr_t)address[1] << SHIFT_1_BYTE;
    result |= (uintptr_t)address[2] << SHIFT_2_BYTE;
    result |= (uintptr_t)address[3] << SHIFT_3_BYTE;
    
    if (address[3] & 0x80)
        result |= (uintptr_t)0xFFFFFFFF00000000;
    
    result &= GC_MASK;
    return result;
}
