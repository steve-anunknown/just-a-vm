#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define GC_MASK      (0x7FFFFFFFFFFFFFFF)
#define SHIFT_1_BYTE (8)
#define SHIFT_2_BYTE (16)
#define SHIFT_3_BYTE (24)
uintptr_t get1Byte(void *ptr);
uintptr_t get2Byte(void *ptr);
uintptr_t get2ByteAddress(void *ptr);
uintptr_t get4Byte(void *ptr);

#endif
