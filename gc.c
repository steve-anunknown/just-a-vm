#include "gc.h"

bool markAndSweep(garbage_collector* gc)
{
    /*
     * the roots array holds the addresses of the cons items,
     * as they are found on the stack.
     */
    uintptr_t*   roots = malloc(sizeof(uintptr_t)*(gc->size));
    unsigned int count = 0;
    // iterate over the stack to find roots
    for (int i = 0; i < gc->machine->top; ++i)
    {
        if (PointsToHeap(gc->machine->data[i])){
            /*
             * gc->machine->data[i] is an address that has its msb marked.
             * gc->machine->data[i] & GC_MASK is the actual address of the cons
             * item in the heap.  In its current form, it cannot be used to
             * index the bit array.  The bottom address of the heap needs to be
             * substracted from it.
             */
            roots[count++] = gc->machine->data[i];
        }
    }
    // mark: dfs for every root
    // sanity check: count how many nodes were marked
    unsigned int marked = 0;
    while (count-- != 0)
    {
        /*
         * first make sure it points to the heap, then get the address.
         */
        if (PointsToHeap(roots[count]))
        {
            /*
             * Here, the bit array is being indexed. Therefore, the bottom
             * address of the heap (gc->bottom) is substracted from the actual
             * address (roots[count]).  If it is marked, just go on. Else, mark
             * it and put it on the dfs stack.
             */
            roots[count] &= GC_MASK;
            if (!IsMarked(gc->bitarray, roots[count] - gc->bottom))
            {
                Mark(gc->bitarray, roots[count] - gc->bottom);
                marked++;
                /*
                 * there is a crucial point here. The cons address that is being
                 * used to access the 'head' and 'tail' fields is first found on
                 * the stack and converted to an actual address. Since it was found
                 * on the stack, it was a marked address. Therefore, the 'head' and
                 * 'tail' fields should be marked as well for the algorithm to work.
                 *
                 * This must be taken care of in the handling of the 'cons' case in
                 * the main loop of the machine.
                 */
                const uintptr_t temp = roots[count];
                roots[count]     = ((cons*)temp)->head;
                roots[count + 1] = (uintptr_t) ((cons*)temp)->tail;
                count += 2;
            }
        }
    }
    // sweep: go through the heap (the bitarray actually) and add unmarked
    // elements to the freelist
    unsigned int freed = 0;
    unsigned int reachable = 0;
    unsigned int index = 0;
    while (index < gc->size)
    {
        if (IsMarked(gc->bitarray, index))
        {
            reachable++;
            Unmark(gc->bitarray, index);
        }
        else
        {
            freed++;
            cons* temp = (cons*)(gc->bottom + index);
            temp->head = (uintptr_t) gc->freelist;
            gc->freelist = temp;
        }
        index += sizeof(cons);
    }
    if (marked != reachable && reachable + freed != gc->size)
    {
        printf("marked: %d, reachable: %d\n", marked, reachable);
        printf("ERROR: marked != reachable\n");
        exit(1);
    }
    free(roots);
    return gc->freelist;
}
