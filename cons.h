#ifndef CONS_H
#define CONS_H

struct _cons_cell
{
    uintptr_t head;
    struct _cons_cell* tail;
};
typedef struct _cons_cell cons;

#endif

