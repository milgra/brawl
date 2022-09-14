
#ifndef gun_h
#define gun_h

#include "cmd.c"
#include "floatbuffer.c"
#include "ogl.c"
#include "physics2.c"
#include <stdio.h>

typedef struct _gun_t gun_t;
struct _gun_t
{
    mass2_t* mass;
    float    skincoords;
    float    trajcoords;
    char     shot;
    char     bullets;
    int8_t   facing;
};

gun_t* gun_alloc(v2_t trans, v2_t basis);
void   gun_dealloc(void* pointer);
void   gun_new(gun_t* gun, surfaces_t* surfaces, v2_t* gravity, cmdqueue_t* actions, float delta);
void   gun_new_skin(gun_t* gun, floatbuffer_t* buffer);

#endif /* gun_h */

#if __INCLUDE_LEVEL__ == 0

/* default state */

gun_t* gun_alloc(v2_t trans, v2_t basis)
{

    gun_t* gun = CAL(sizeof(gun_t), gun_dealloc, NULL);

    gun->mass        = mass2_alloc(trans, 4.0, 5.0, .2);
    gun->mass->basis = v2_scale(basis, 2.0);
    gun->bullets     = 6;

    return gun;
}

/* cleanup */

void gun_dealloc(void* pointer)
{

    gun_t* gun = pointer;

    REL(gun->mass);
}

/* new state */

void gun_new(gun_t* gun, surfaces_t* surfaces, v2_t* gravity, cmdqueue_t* actions, float delta)
{

    gun->mass->basis = v2_add(gun->mass->basis, *gravity);
    physics2_new_mass_position(gun->mass, surfaces, delta);

    if (gun->mass->trans.y < -10000.0)
    {
	// TODO!!!
	cmdqueue_add(actions, "scene.removegun", NULL, gun);
    }
}

/* new skin state */

void gun_new_skin(gun_t* gun, floatbuffer_t* buffer)
{

    if (gun->skincoords == 0) gun->skincoords = ogl_color_float_from_rgbauint32(0x000000FF);
    if (gun->trajcoords == 0) gun->trajcoords = ogl_color_float_from_rgbauint32(0xFFFF00FF);

    v2_t gunhor40  = v2_resize(gun->mass->basis, 40.0);
    v2_t gunhor10  = v2_resize(gun->mass->basis, 10.0);
    v2_t gunhor15  = v2_resize(gun->mass->basis, 15.0);
    v2_t gunhorm5  = v2_resize(gun->mass->basis, -5.0);
    v2_t gunvert20 = gun->facing > 0 ? v2_rotate_90_right(gunhor40) : v2_rotate_90_left(gunhor40);
    gunvert20      = v2_resize(gunvert20, 20.0);
    v2_t gunvert5  = v2_resize(gunvert20, 5.0);

    v2_t trans = gun->mass->trans;
    trans      = v2_add(trans, v2_resize(gunvert5, -10.0));

    floatbuffer_addvector2(buffer, trans);
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, gunhor40));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, v2_add(gunhor40, gunvert5)));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, trans);
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, v2_add(gunhor40, gunvert5)));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, gunvert5));
    floatbuffer_add(buffer, gun->skincoords);

    floatbuffer_addvector2(buffer, trans);
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, gunhor15));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, v2_add(gunhor10, gunvert20)));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, trans);
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, v2_add(gunhor10, gunvert20)));
    floatbuffer_add(buffer, gun->skincoords);
    floatbuffer_addvector2(buffer, v2_add(trans, v2_add(gunhorm5, gunvert20)));
    floatbuffer_add(buffer, gun->skincoords);

    if (gun->shot == 1)
    {
	gun->shot  = 0;
	v2_t depth = v2_resize(v2_rotate_90_right(gun->mass->basis), 5.0);
	floatbuffer_addvector2(buffer, trans);
	floatbuffer_add(buffer, gun->trajcoords);
	floatbuffer_addvector2(buffer, v2_add(v2_add(trans, v2_init(gun->facing * 40.0, 0.0)), gun->mass->basis));
	floatbuffer_add(buffer, gun->trajcoords);
	floatbuffer_addvector2(buffer, v2_add(v2_add(trans, gun->mass->basis), depth));
	floatbuffer_add(buffer, gun->trajcoords);
	floatbuffer_addvector2(buffer, v2_add(v2_add(trans, v2_init(gun->facing * 40.0, 0.0)), v2_init(gun->facing * 10.0, 0.0)));
	floatbuffer_add(buffer, gun->trajcoords);
	floatbuffer_addvector2(buffer, v2_add(v2_add(trans, gun->mass->basis), depth));
	floatbuffer_add(buffer, gun->trajcoords);
	floatbuffer_addvector2(buffer, v2_add(v2_add(trans, v2_init(gun->facing * 40.0, 0.0)), depth));
	floatbuffer_add(buffer, gun->trajcoords);
    }
}

#endif
