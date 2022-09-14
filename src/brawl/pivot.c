
#ifndef parabola_types_h
#define parabola_types_h

#include "math2.c"

typedef struct _pivot_t pivot_t;
struct _pivot_t
{

    char* id;
    v2_t  position;
};

pivot_t* pivot_alloc(char* id, v2_t position);

void pivot_destruct(void* pointer);

#endif /* parabola_types_h */

#if __INCLUDE_LEVEL__ == 0

#include "mtcstr.c"
#include "mtmem.c"
#include "mtstr.c"

void pivot_destruct(void* pointer)
{

    pivot_t* pivot = pointer;
    mtmem_release(pivot->id);
}

pivot_t* pivot_alloc(char* id, v2_t position)
{

    pivot_t* pivot = mtmem_alloc(sizeof(pivot_t), pivot_destruct);

    pivot->id       = mtcstr_fromcstring(id);
    pivot->position = position;

    return pivot;
}

#endif
