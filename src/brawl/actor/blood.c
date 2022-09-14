
#ifndef blood_h
#define blood_h

#include "cmd.c"
#include "floatbuffer.c"
#include "math2.c"
#include "ogl.c"
#include "physics2.c"
#include <stdio.h>

typedef struct _blood_t blood_t;
struct _blood_t
{
    mass2_t* mass;
    float    skincoords;
};

blood_t* blood_alloc(v2_t trans, v2_t basis);
void     blood_dealloc(void* pointer);
void     blood_new(blood_t* blood, surfaces_t* surfaces, v2_t* gravity, cmdqueue_t* actions, float delta);
void     blood_new_skin(blood_t* blood, floatbuffer_t* buffer);

#endif /* blood_h */

#if __INCLUDE_LEVEL__ == 0

/* default state */

blood_t* blood_alloc(v2_t trans, v2_t basis)
{
    blood_t* blood = mtmem_calloc(sizeof(blood_t), blood_dealloc);

    blood->mass        = mass2_alloc(trans, 4.0, 5.0, .2);
    blood->mass->basis = v2_scale(basis, 2.0);

    return blood;
}

/* cleanup */

void blood_dealloc(void* pointer)
{
    blood_t* blood = pointer;

    mtmem_release(blood->mass);
}

/* new state */

void blood_new(blood_t* blood, surfaces_t* surfaces, v2_t* gravity, cmdqueue_t* actions, float delta)
{
    blood->mass->basis = v2_add(blood->mass->basis, *gravity);
    physics2_new_mass_position(blood->mass, surfaces, delta);

    if (blood->mass->trans.y < -10000.0)
    {
	cmdqueue_add(actions, "scene.removeblood", NULL, blood);
    }
}

/* new skin state */

void blood_new_skin(blood_t* blood, floatbuffer_t* buffer)
{
    if (blood->skincoords == 0) blood->skincoords = ogl_color_float_from_rgbauint32(0xFF0000FF);

    floatbuffer_addvector2(buffer, blood->mass->trans);
    floatbuffer_add(buffer, blood->skincoords);
}

#endif
