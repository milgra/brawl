
#ifndef actor_skin_h
#define actor_skin_h

#include "actor.c"
#include "floatbuffer.c"

typedef struct _actor_skin_t actor_skin_t;
struct _actor_skin_t
{
    float randoms[4][22];
    float acoords;
    float bcoords;
    float ccoords;
    float dcoords;
    float hcoords;
    float wcoords;
};

actor_skin_t* actor_skin_alloc(void);
void          actor_skin_new(actor_skin_t* skin, actor_t* actor, floatbuffer_t* buffer, uint8_t skinphase);

#endif /* actor_skin_h */

#if __INCLUDE_LEVEL__ == 0

/* default state */

actor_skin_t* actor_skin_alloc()
{
    actor_skin_t* skin = mtmem_calloc(sizeof(actor_skin_t), NULL);

    /* generate random values */

    for (uint8_t phase = 0; phase < 4; phase++)
    {
	for (uint8_t index = 0; index < 22; index++) skin->randoms[phase][index] = -1.5 + (float) (rand() % 30) / 10.0;
    }

    return skin;
}

/* head trinagles */

void actor_skin_triangles_for_head(
    actor_t*       actor,
    v2_t           point_a,
    v2_t           point_b,
    float          coords,
    floatbuffer_t* buffer,
    float          stroke,
    int8_t         facing)
{
    if (stroke > 0.0)
    {
	v2_t ba = v2_sub(point_a, point_b);
	ba      = v2_resize(ba, v2_length(ba) + 5.0);
	point_a = v2_add(point_b, ba);
    }

    v2_t ab = v2_sub(point_b, point_a);

    v2_t nlsa = v2_add(point_a, v2_resize(v2_rotate_90_left(ab), 10.0 + stroke));
    v2_t nrsa = v2_add(point_a, v2_resize(v2_rotate_90_right(ab), 10.0 + stroke));

    v2_t nlea = v2_add(point_b, v2_resize(v2_rotate_90_left(ab), 11.0 + stroke));
    v2_t nrea = v2_add(point_b, v2_resize(v2_rotate_90_right(ab), 11.0 + stroke));

    v2_t ab23 = v2_add(point_a, v2_scale(ab, 0.66));
    v2_t nose;

    if (facing == 1) nose = v2_add(ab23, v2_resize(v2_rotate_90_left(ab), 15.0 + stroke));
    else nose = v2_add(ab23, v2_resize(v2_rotate_90_right(ab), 15.0 + stroke));

    if (facing == 1)
    {
	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nose);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nose);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nlea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);
    }
    else
    {
	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nose);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nose);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nlea);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nose);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nlea);
	floatbuffer_add(buffer, coords);
    }
}

/* foot triangles */

void actor_skin_triangles_for_foot(
    actor_t*       actor,
    v2_t           point_a,
    v2_t           point_b,
    float          coords,
    floatbuffer_t* buffer,
    float          stroke)
{
    v2_t ab    = v2_sub(point_b, point_a);
    v2_t abbig = v2_add(point_a, v2_scale(ab, 1.2));
    v2_t toppoint;
    v2_t leftpoint;
    v2_t rightpoint;

    if (actor->facing == kActorFacingRight)
    {
	leftpoint  = v2_add(abbig, v2_resize(v2_rotate_90_right(ab), 10.0 + stroke));
	rightpoint = v2_add(abbig, v2_resize(v2_rotate(ab, M_PI / 2.0), 20.0 + stroke));
	toppoint   = v2_add(leftpoint, v2_resize(ab, -18.0 - stroke));
    }
    else
    {
	leftpoint  = v2_add(abbig, v2_resize(v2_rotate(ab, -M_PI / 2.0), 20.0 + stroke));
	rightpoint = v2_add(abbig, v2_resize(v2_rotate_90_left(ab), 10.0 + stroke));
	toppoint   = v2_add(rightpoint, v2_resize(ab, -18.0 - stroke));
    }

    floatbuffer_addvector2(buffer, toppoint);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, leftpoint);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, rightpoint);
    floatbuffer_add(buffer, coords);
}

/* triangles for point chain */

void actor_skin_triangles_for_points(
    actor_t*       actor,
    v2_t*          points,
    float*         sizes,
    float          coords,
    uint32_t       count,
    floatbuffer_t* buffer)
{
    for (int index = 0; index < count - 2; index++)
    {
	v2_t point_a = points[index];
	v2_t point_b = points[index + 1];
	v2_t point_c = points[index + 2];

	v2_t ab = v2_sub(point_b, point_a);
	v2_t bc = v2_sub(point_c, point_b);

	v2_t nlsa = v2_add(point_a, v2_resize(v2_rotate_90_left(ab), sizes[index]));
	v2_t nrsa = v2_add(point_a, v2_resize(v2_rotate_90_right(ab), sizes[index]));

	v2_t nlea = v2_add(point_b, v2_resize(v2_rotate_90_left(ab), sizes[index + 1]));
	v2_t nrea = v2_add(point_b, v2_resize(v2_rotate_90_right(ab), sizes[index + 1]));

	v2_t nlsb = v2_add(point_b, v2_resize(v2_rotate_90_left(bc), sizes[index + 1]));
	v2_t nrsb = v2_add(point_b, v2_resize(v2_rotate_90_right(bc), sizes[index + 1]));

	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nlsa);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nlea);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nlea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nlsb);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, point_b);
	floatbuffer_add(buffer, coords);

	floatbuffer_addvector2(buffer, nrea);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, nrsb);
	floatbuffer_add(buffer, coords);
	floatbuffer_addvector2(buffer, point_b);
	floatbuffer_add(buffer, coords);
    }

    /* ending quad */

    v2_t point_a = points[count - 2];
    v2_t point_b = points[count - 1];

    v2_t ab = v2_sub(point_b, point_a);

    v2_t nlsa = v2_add(point_a, v2_resize(v2_rotate_90_left(ab), sizes[count - 2]));
    v2_t nrsa = v2_add(point_a, v2_resize(v2_rotate_90_right(ab), sizes[count - 2]));

    v2_t nlea = v2_add(point_b, v2_resize(v2_rotate_90_left(ab), sizes[count - 1]));
    v2_t nrea = v2_add(point_b, v2_resize(v2_rotate_90_right(ab), sizes[count - 1]));

    floatbuffer_addvector2(buffer, nlsa);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, nrsa);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, nrea);
    floatbuffer_add(buffer, coords);

    floatbuffer_addvector2(buffer, nlsa);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, nrea);
    floatbuffer_add(buffer, coords);
    floatbuffer_addvector2(buffer, nlea);
    floatbuffer_add(buffer, coords);
}

/* new state */

void actor_skin_new(actor_skin_t* skin, actor_t* actor, floatbuffer_t* buffer, uint8_t skinphase)
{
    /* request coords first */

    if (skin->acoords == 0.0)
    {
	skin->acoords = ogl_color_float_from_rgbauint32(actor->metrics.color_a);
	skin->bcoords = ogl_color_float_from_rgbauint32(actor->metrics.color_b);
	skin->ccoords = ogl_color_float_from_rgbauint32(actor->metrics.color_c);
	skin->dcoords = ogl_color_float_from_rgbauint32(actor->metrics.color_d);
	skin->hcoords = ogl_color_float_from_rgbauint32(0xAA8888FE);
	skin->wcoords = ogl_color_float_from_rgbauint32(0x000000FE);
    }

    /* modify actor points based on phase */

    actor_points_t points  = actor->points;
    float*         randoms = skin->randoms[skinphase];

    v2_t legpoints_a[] = {
	points.neck,
	points.hip,
	points.knee_a,
	points.ankle_a};

    v2_t legpoints_b[] = {
	points.neck,
	points.hip,
	points.knee_b,
	points.ankle_b};

    float legsizes[] = {
	1.0,
	actor->metrics.hipwidth + randoms[0],
	actor->metrics.legwidth * .7 + randoms[1],
	actor->metrics.legwidth + randoms[2]};

    float legsizes1[] = {
	1.0 + 5.0,
	actor->metrics.hipwidth + 5.0 + randoms[3],
	actor->metrics.legwidth * .7 + 5.0 + randoms[4],
	actor->metrics.legwidth + 5.0 + randoms[5]};

    actor_skin_triangles_for_foot(actor, points.knee_a, points.ankle_a, skin->wcoords, buffer, 5.0 + randoms[6]);
    actor_skin_triangles_for_points(actor, legpoints_a, legsizes1, skin->wcoords, 4, buffer);
    actor_skin_triangles_for_points(actor, legpoints_a, legsizes, skin->dcoords, 4, buffer);
    actor_skin_triangles_for_foot(actor, points.knee_a, points.ankle_a, skin->dcoords, buffer, 0.0);

    actor_skin_triangles_for_foot(actor, points.knee_b, points.ankle_b, skin->wcoords, buffer, 5.0 + randoms[7]);
    actor_skin_triangles_for_points(actor, legpoints_b, legsizes1, skin->wcoords, 4, buffer);
    actor_skin_triangles_for_points(actor, legpoints_b, legsizes, skin->ccoords, 4, buffer);
    actor_skin_triangles_for_foot(actor, points.knee_b, points.ankle_b, skin->ccoords, buffer, 0.0);

    v2_t armpoints_a[] = {
	points.neck,
	points.elbow_a,
	points.hand_a};

    v2_t armpoints_b[] = {
	points.neck,
	points.elbow_b,
	points.hand_b};

    float armsizes[] = {
	actor->metrics.armwidth + randoms[8],
	actor->metrics.armwidth * .9 + randoms[9],
	actor->metrics.armwidth * .8 + randoms[10]};

    float armsizes1[] = {
	actor->metrics.armwidth + 5.0 + randoms[11],
	actor->metrics.armwidth * .9 + 5.0 + randoms[12],
	actor->metrics.armwidth * .8 + 5.0 + randoms[13]};

    actor_skin_triangles_for_points(actor, armpoints_a, armsizes1, skin->wcoords, 3, buffer);
    actor_skin_triangles_for_points(actor, armpoints_a, armsizes, skin->bcoords, 3, buffer);

    v2_t bodypoints[] = {
	points.head,
	points.neck,
	points.hip};

    float bodysizes[] = {
	actor->metrics.neckwidth + randoms[14],
	actor->metrics.neckwidth + randoms[15],
	actor->metrics.hipwidth + randoms[16]};

    float bodysizes1[] = {
	actor->metrics.neckwidth + 5.0 + randoms[17],
	actor->metrics.neckwidth + 5.0 + randoms[18],
	actor->metrics.hipwidth + 5.0 + randoms[19],
    };

    actor_skin_triangles_for_points(actor, bodypoints, bodysizes1, skin->wcoords, 3, buffer);
    actor_skin_triangles_for_points(actor, bodypoints, bodysizes, skin->acoords, 3, buffer);

    actor_skin_triangles_for_head(actor, points.head, points.neck, skin->wcoords, buffer, 5.0 + randoms[20], actor->facing);
    actor_skin_triangles_for_head(actor, points.head, points.neck, skin->hcoords, buffer, 0.0, actor->facing);

    actor_skin_triangles_for_points(actor, armpoints_b, armsizes1, skin->wcoords, 3, buffer);
    actor_skin_triangles_for_points(actor, armpoints_b, armsizes, skin->acoords, 3, buffer);
}

#endif
