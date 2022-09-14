
#ifndef actor_modifier_kick_h
#define actor_modifier_kick_h

#include "math1.c"
#include "math2.c"
#include "mtmem.c"
#include <stdio.h>
#include <stdlib.h>
#include "actor_modifier_types.c"

typedef struct _actor_modifier_kick_t actor_modifier_kick_t;
struct _actor_modifier_kick_t
{
    char phase;

    v2_t target;

    v2_t* activefoot;
    v2_t* passivefoot;

    v2_t kicktrans;
    v2_t kickbasis;
    v2_t activebasis;
};

actor_modifier_t* actor_modifier_kick_alloc(void);
void              actor_modifier_kick_new(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_kick_destruct(void* pointer);
void              actor_modifier_kick_stopkick(actor_modifier_t* modifier, void* actor);

#endif /* actor_modifier_kick_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_modifier_kick.c"
#include "actor.c"

/* default state */

actor_modifier_t* actor_modifier_kick_alloc()
{
    actor_modifier_t*      modifier = mtmem_calloc(sizeof(actor_modifier_t), actor_modifier_kick_destruct);
    actor_modifier_kick_t* data     = mtmem_calloc(sizeof(actor_modifier_kick_t), NULL);

    modifier->data = data;
    modifier->_new = actor_modifier_kick_new;

    data->phase     = 0;
    data->target    = v2_init(0.0, 0.0);
    data->kicktrans = v2_init(0.0, 0.0);
    data->kickbasis = v2_init(0.0, 0.0);

    return modifier;
}

/* cleanup */

void actor_modifier_kick_destruct(void* pointer)
{
    actor_modifier_t* modifier = pointer;
    mtmem_release(modifier->data);
}

/* inits kick animation */

void actor_modifier_kick_initkick(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_kick_t* data  = modifier->data;

    if (args->control_state.kick_pressed == 1 &&   /* kick is pressed */
	actor->control_state.kick_pressed == 0 &&  /* kick was not pressed before */
	actor->control_state.squat_pressed == 0 && /* not squatting */
	actor->control_state.block_pressed == 0 && /* not squatting */
	actor->state == kActorStateWalk &&         /* state is walk */
	actor->gothit == 0 &&                      /* we are not in hit state */
	actor->power >= actor->metrics.hitpower)   /* hitting power is over 0.4 */
    {
	/* reduce hitting power */

	actor->power -= actor->metrics.hitpower;

	/* set ankles */

	actor->points.ankle_a = actor->points.base_a;
	actor->points.ankle_b = actor->points.base_b;

	/* set active and passive foot */

	if ((actor->facing == kActorFacingRight && (actor->points.base_b.x > actor->points.base_a.x)) ||
	    (actor->facing == kActorFacingLeft && (actor->points.base_b.x < actor->points.base_a.x)))
	{
	    data->activefoot  = &actor->points.ankle_b;
	    data->passivefoot = &actor->points.ankle_a;
	}
	else
	{
	    data->activefoot  = &actor->points.ankle_a;
	    data->passivefoot = &actor->points.ankle_b;
	}

	/* set state */

	actor->state      = kActorStateKick;
	data->activebasis = *data->activefoot;

	/* set activefoot's target position */

	float angle = ((float) (rand() % 1000) / 1000.0) * M_PI / 4.0 - M_PI / 12.0;

	data->target.x = actor->points.hip.x + actor->facing * cosf(angle) * actor->metrics.leglength * 1.5;
	data->target.y = actor->points.hip.y + sinf(angle) * actor->metrics.leglength * 1.5;

	data->kicktrans = *(data->passivefoot);
	data->phase     = 0;
    }
}

/* stops kick if kicking */

void actor_modifier_kick_stopkick(actor_modifier_t* modifier, void* actorp)
{
    actor_t*               actor = actorp;
    actor_modifier_kick_t* data  = modifier->data;

    if (data->phase == 0)
    {
	data->phase = 1;

	data->kickbasis = v2_sub(data->target, *data->passivefoot);
	data->target    = data->kicktrans;
    }
    else actor->state = kActorStateWalk;
}

/* new hand positions in kick state */

void actor_modifier_kick_points_new_hands(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t* actor = args->actor;

    float neckx = actor->points.hip.x + actor->facing * fabs(actor->points.base_b.x - actor->points.base_a.x) / 8.0;
    float necky = actor->points.hip.y + actor->metrics.bodylength;

    actor->points.neck = v2_init(neckx, necky);
    actor->points.head = v2_init(neckx, necky + actor->metrics.headlength);

    float handax = actor->facing * (actor->metrics.armlength * .4 + (actor->points.base_b.x - actor->points.base_a.x) / 8.0);
    float handbx = actor->facing * (actor->metrics.armlength * .4 - (actor->points.base_b.x - actor->points.base_a.x) / 8.0);
    float handay = -actor->metrics.armlength * 0.1;
    float handby = -actor->metrics.armlength * 0.14;

    actor->points.hand_a = v2_init(actor->points.neck.x + handax, actor->points.neck.y + handay);
    actor->points.hand_b = v2_init(actor->points.neck.x + handbx, actor->points.neck.y + handby);

    actor->points.elbow_b = v2_triangle_with_bases(actor->points.neck, actor->points.hand_b, actor->metrics.armlength * .5, -1 * actor->facing);
    actor->points.elbow_a = v2_triangle_with_bases(actor->points.neck, actor->points.hand_a, actor->metrics.armlength * .5, -1 * actor->facing);

    v2_t ankle_a = actor->points.ankle_a;
    v2_t ankle_b = actor->points.ankle_b;

    actor->points.knee_b = v2_triangle_with_bases(actor->points.hip, ankle_b, actor->metrics.leglength * .5, actor->facing);
    actor->points.knee_a = v2_triangle_with_bases(actor->points.hip, ankle_a, actor->metrics.leglength * .5, actor->facing);
}

/* new modifier state */

void actor_modifier_kick_new(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_kick_t* data  = modifier->data;

    if (args->type == NULL)
    {
	actor_modifier_kick_initkick(modifier, args);

	if (actor->state == kActorStateKick)
	{
	    /* get distance to target in actual phase */

	    v2_t  dir    = v2_sub(data->target, *data->passivefoot);
	    float length = v2_length(dir);

	    if (length > 10.0)
	    {
		dir = v2_resize(dir, 10.0 * args->delta);

		*data->passivefoot = v2_add(*data->passivefoot, dir);
		*data->activefoot  = data->activebasis;

		actor->points.hip.x += dir.x / 5.0;
		actor->points.hip.y = data->activefoot->y + actor->metrics.leglength * 0.9;
	    }
	    else
	    {
		if (data->phase == 0)
		{
		    if (actor->gothit == 0)
		    {
			attack_t* attack = attack_alloc(actor, actor->points.hip, v2_sub(*data->passivefoot, actor->points.hip), actor->metrics.hitpower);

			cmdqueue_add(args->cmdqueue, "scene.kick", NULL, attack);

			mtmem_release(attack);
		    }

		    data->phase = 1;

		    data->kickbasis = v2_sub(data->target, *data->passivefoot);
		    data->target    = data->kicktrans;
		}
		else
		{
		    actor->state                      = kActorStateWalk;
		    args->control_state.block_pressed = 0;
		}
	    }

	    actor_modifier_kick_points_new_hands(modifier, args);
	}
    }
}

#endif
