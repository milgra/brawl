/* Pivot points in the scene */

#ifndef parabola_types_h
#define parabola_types_h

#include "zc_util2.c"

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

#include "zc_cstring.c"
#include "zc_memory.c"
#include "zc_string.c"

void pivot_destruct(void* pointer)
{
    pivot_t* pivot = pointer;
    REL(pivot->id);
}

pivot_t* pivot_alloc(char* id, v2_t position)
{
    pivot_t* pivot = CAL(sizeof(pivot_t), pivot_destruct, NULL);

    pivot->id       = cstr_new_cstring(id);
    pivot->position = position;

    return pivot;
}

#endif
