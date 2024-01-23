#ifndef BITARRAY_H
#define BITARRAY_H

#define MARK_FAKE        (0x8000000000000000)
#define Mark(A,k)        ( A[(k)/32] |= (1 << ((k)%32))  )
#define Unmark(A,k)      ( A[(k)/32] &= ~(1 << ((k)%32)) )
#define IsMarked(A,k)    ( A[(k)/32] & (1 << ((k)%32))   )
#define PointsToHeap(a)  ( (a & (MARK_FAKE)) )

#endif