struct _cons_cell
{
    intptr_t head;
    struct _cons_cell *tail;
};
typedef struct _cons_cell cons;

