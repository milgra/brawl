#ifndef actor_debug_h
#define actor_debug_h

#include "actor.c"
#include "floatbuffer.c"

void actor_debug_lines(actor_t* actor, floatbuffer_t* linebuffer);
void actor_debug_points(actor_t* actor, floatbuffer_t* buffer);

#endif /* actor_debug_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_debug.c"
#include "actor_modifier_jump.c"
#include "actor_modifier_kick.c"
#include "actor_modifier_ragdoll.c"
#include "actor_modifier_walk.c"

/* adds point to debug point buffer */

void actor_debug_add_line(floatbuffer_t* buffer, v2_t point_a, v2_t point_b, float coord)
{
    floatbuffer_addvector2(buffer, point_a);
    floatbuffer_add(buffer, coord);
    floatbuffer_addvector2(buffer, point_b);
    floatbuffer_add(buffer, coord);
}

/* new modifier state */

void actor_debug_lines(actor_t* actor, floatbuffer_t* linebuffer)
{
    float coords = ogl_color_float_from_rgbauint32(0xFFFFFFFF);

    /* bones */

    actor_debug_add_line(linebuffer, actor->points.head, actor->points.neck, coords);
    actor_debug_add_line(linebuffer, actor->points.neck, actor->points.hip, coords);
    actor_debug_add_line(linebuffer, actor->points.hip, actor->points.knee_a, coords);
    actor_debug_add_line(linebuffer, actor->points.knee_a, actor->points.ankle_a, coords);
    actor_debug_add_line(linebuffer, actor->points.hip, actor->points.knee_b, coords);
    actor_debug_add_line(linebuffer, actor->points.knee_b, actor->points.ankle_b, coords);
    actor_debug_add_line(linebuffer, actor->points.neck, actor->points.elbow_a, coords);
    actor_debug_add_line(linebuffer, actor->points.elbow_a, actor->points.hand_a, coords);
    actor_debug_add_line(linebuffer, actor->points.neck, actor->points.elbow_b, coords);
    actor_debug_add_line(linebuffer, actor->points.elbow_b, actor->points.hand_b, coords);

    /* sight */

    actor_modifier_walk_t* data = actor->walk->data;

    actor_debug_add_line(linebuffer, data->sight.trans, v2_add(data->sight.trans, data->sight.basis_lower), coords);
    actor_debug_add_line(linebuffer, data->sight.trans, v2_add(data->sight.trans, data->sight.basis_upper), coords);

    /* attack */

    actor_modifier_ragdoll_t* ikdata = actor->ik->data;

    actor_debug_add_line(linebuffer, ikdata->attack.trans, v2_add(ikdata->attack.trans, ikdata->attack.basis), coords);

    /* kick */

    actor_modifier_kick_t* kickdata = actor->kick->data;

    actor_debug_add_line(linebuffer, actor->points.hip, v2_add(actor->points.hip, kickdata->kickbasis), coords);
}

/* points */

void actor_debug_points(actor_t* actor, floatbuffer_t* pointbuffer)
{
    float coords = ogl_color_float_from_rgbauint32(0xFFFFFFFF);

    /* joints */

    floatbuffer_addvector2(pointbuffer, actor->points.base_a);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.base_b);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.head);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.neck);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.hip);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.knee_a);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.knee_b);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.ankle_a);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.ankle_b);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.elbow_a);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.hand_a);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.elbow_b);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, actor->points.hand_b);
    floatbuffer_add(pointbuffer, coords);

    /* sight */

    actor_modifier_walk_t* data = actor->walk->data;

    floatbuffer_addvector2(pointbuffer, data->sight.final_point);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, data->sight.trans);
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, v2_add(data->sight.trans, data->sight.basis_lower));
    floatbuffer_add(pointbuffer, coords);
    floatbuffer_addvector2(pointbuffer, v2_add(data->sight.trans, data->sight.basis_upper));
    floatbuffer_add(pointbuffer, coords);

    /* hitpoint */

    actor_modifier_ragdoll_t* ikdata = actor->ik->data;

    floatbuffer_addvector2(pointbuffer, ikdata->hitpoint);
    floatbuffer_add(pointbuffer, coords);
}

#endif
