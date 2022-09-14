
#ifndef actor_modifier_walk_h
#define actor_modifier_walk_h

#include "actor_modifier_walk_points.c"
#include "math1.c"
#include "zc_memory.c"
#include <stdio.h>
#include <stdlib.h>

typedef struct _actor_modifier_walk_t actor_modifier_walk_t;
struct _actor_modifier_walk_t
{
    char is_moving;
    char wants_to_jump;
    char vertical_direction;

    float maxspeed;
    float prevspeed;
    float steplength;
    float squatsize;
    float breathangle;

    sight_t sight;

    movement_t block;
    movement_t puncha;
    movement_t punchb;

    v2_t* activebase;
    v2_t* passivebase;
};

actor_modifier_t* actor_modifier_walk_alloc(void);
void              actor_modifier_walk_new(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_walk_destruct(void* pointer);
void              actor_modifier_walk_sendpunchaction(actor_modifier_t* modifier, actor_modifier_args_t* args, v2_t basis);
void              actor_modifier_walk_stoppunch(actor_modifier_t* modifier);

#endif /* actor_modifier_walk_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_modifier_walk.c"
#include <string.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/* default state */

actor_modifier_t* actor_modifier_walk_alloc()
{
    actor_modifier_t*      modifier = CAL(sizeof(actor_modifier_t), actor_modifier_walk_destruct, NULL);
    actor_modifier_walk_t* data     = CAL(sizeof(actor_modifier_walk_t), NULL, NULL);

    modifier->data = data;
    modifier->_new = actor_modifier_walk_new;

    data->is_moving          = 0;
    data->wants_to_jump      = 0;
    data->vertical_direction = 0;

    data->maxspeed    = 0.0;
    data->prevspeed   = 0.0;
    data->squatsize   = 0.0;
    data->steplength  = 0.0;
    data->breathangle = 0.0;

    /* these will be set on first left/right */

    data->activebase  = NULL;
    data->passivebase = NULL;

    return modifier;
}

/* cleanup */

void actor_modifier_walk_destruct(void* pointer)
{
    actor_modifier_t* modifier = pointer;

    REL(modifier->data);
}

/* sends punch action to controller */

void actor_modifier_walk_sendpunchaction(actor_modifier_t* modifier, actor_modifier_args_t* args, v2_t basis)
{
    actor_t* actor = args->actor;

    attack_t* attack = attack_alloc(actor, actor->points.neck, basis, actor->metrics.hitpower);

    cmdqueue_add(args->cmdqueue, "scene.punch", NULL, attack);

    REL(attack);
}

/* stops punch movement */

void actor_modifier_walk_stoppunch(actor_modifier_t* modifier)
{
    actor_modifier_walk_t* walk = modifier->data;

    if (walk->puncha.speed > 0) walk->puncha.speed *= -1;
    if (walk->punchb.speed > 0) walk->punchb.speed *= -1;
}

/* check ground under active base */

void actor_modifier_walk_check_ground(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_walk_t* data  = modifier->data;

    v2_t trans = v2_init(data->activebase->x, data->activebase->y + 20.0);
    v2_t basis = v2_init(0.0, -100.0);

    /* look for intersecting segment */

    physics2_collision_t collision = {0};
    physics2_collect_intersecting_surfaces(args->surfaces, trans, basis, 10.0, &collision);

    /* no ground for leg, collapse like a dried figs */

    if (collision.count == 0)
    {
	actor->state = kActorStateDeath;
	printf("DEATH BY NO GROUND");
	if (strcmp(actor->name, "hero") == 0)
	{
	    str_t* text = str_frombytes("No ground for foot");
	    cmdqueue_delay(args->cmdqueue, "scene.showwasted", text, NULL, args->ticks + 180);
	    REL(text);
	}
    }
}

/* adjust speed */

void actor_modifier_walk_speed(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_walk_t* data  = modifier->data;

    /* set max speed */

    if (data->maxspeed == 0)
    {
	data->maxspeed = actor->metrics.walkspeed;
    }
    if (args->control_state.run_pressed == 1 && actor->control_state.run_pressed == 0)
    {
	data->maxspeed = actor->metrics.runspeed;
    }
    if (args->control_state.run_pressed == 0 && data->maxspeed > actor->metrics.walkspeed)
    {
	data->maxspeed = actor->metrics.walkspeed;
    }
    if (args->control_state.squat_pressed == 1)
    {
	data->maxspeed = actor->metrics.walkspeed;
    }
    if (args->control_state.punch_pressed == 1 || args->control_state.kick_pressed == 1)
    {
	data->maxspeed = 0.0;
    }
    if (args->control_state.block_pressed == 1 && actor->name[0] == 'h')
    {
	if (actor->facing == kActorFacingRight && args->control_state.right_pressed == 1) data->maxspeed = 0.0;
	if (actor->facing == kActorFacingLeft && args->control_state.left_pressed == 1) data->maxspeed = 0.0;
    }

    /* go right or left, slow down/speed up softly if needed */

    if (args->control_state.right_pressed == 1)
    {

	if (actor->speed.x > data->maxspeed)
	{
	    actor->speed.x -= 0.3 * args->delta;
	}
	else
	{
	    actor->speed.x += 0.3 * args->delta;
	    if (actor->speed.x > data->maxspeed) actor->speed.x = data->maxspeed;
	}
    }
    else if (args->control_state.left_pressed == 1)
    {

	if (actor->speed.x < -data->maxspeed)
	{
	    actor->speed.x += 0.3 * args->delta;
	}
	else
	{
	    actor->speed.x -= 0.3 * args->delta;
	    if (actor->speed.x < -data->maxspeed) actor->speed.x = -data->maxspeed;
	}
    }
    else actor->speed.x *= 1.0 - 0.08 * args->delta;

    /* reset target in case of direction change */

    if ((actor->speed.x > 0 && data->prevspeed < 0) || (actor->speed.x < 0 && data->prevspeed > 0))
    {
	data->is_moving = 0;
    }

    data->prevspeed = actor->speed.x;

    /* set actor facing */

    if (actor->speed.x > 0.0)
    {
	if (args->control_state.right_pressed == 1 && args->control_state.block_pressed == 0) actor->facing = kActorFacingRight;
    }
    else
    {
	if (args->control_state.left_pressed == 1 && args->control_state.block_pressed == 0) actor->facing = kActorFacingLeft;
    }
}

/* get new foot target */

void actor_modifier_walk_newtarget(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_walk_t* data  = modifier->data;

    if (data->is_moving == 0 && fabs(actor->speed.x) > 0.1)
    {
	data->is_moving = 1;

	/* get active and passive bases */

	if ((actor->points.base_a.x < actor->points.base_b.x && actor->speed.x >= 0.0) ||
	    (actor->points.base_a.x > actor->points.base_b.x && actor->speed.x < 0.0))
	{
	    data->activebase  = &(actor->points.base_a);
	    data->passivebase = &(actor->points.base_b);
	}
	else
	{
	    data->activebase  = &(actor->points.base_b);
	    data->passivebase = &(actor->points.base_a);
	}

	/* calculate stepsize */

	float stepsize = actor->speed.x / fabs(actor->speed.x) * 40.0 + actor->speed.x * 8.0;

	/* update sight */

	data->sight.trans       = v2_init(data->passivebase->x + stepsize, data->passivebase->y);
	data->sight.basis_upper = v2_init(-stepsize, fabs(stepsize) / 2.0);
	data->sight.basis_lower = v2_init(-stepsize, -fabs(stepsize) / 2.0);

	/* get sight-ground isp */

	v2_t       upper = data->sight.basis_upper;
	v2_t       lower = data->sight.basis_lower;
	v2_t       isp   = {0};
	segment2_t segment;

	physics2_collision_t collision = {0};

	physics2_collect_intersecting_surfaces(args->surfaces, data->sight.trans, upper, 10.0, &collision);
	physics2_collect_intersecting_surfaces(args->surfaces, data->sight.trans, lower, 10.0, &collision);

	if (collision.count == 0) isp = data->sight.trans;
	else
	{
	    isp     = collision.isps[0];
	    segment = collision.segments[0];
	    if (data->vertical_direction == 0)
	    {
		/* get lowest point */

		for (int index = 0; index < collision.count; index++)
		{
		    if (collision.isps[index].y < isp.y)
		    {
			isp     = collision.isps[index];
			segment = collision.segments[index];
		    }
		}
	    }
	    else if (data->vertical_direction == 1)
	    {
		/* get highest point */

		for (int index = 0; index < collision.count; index++)
		{
		    if (collision.isps[index].y > isp.y)
		    {
			isp     = collision.isps[index];
			segment = collision.segments[index];
		    }
		}
	    }
	}

	/* set final point */

	if (collision.count > 0)
	{
	    float angle = fabs(v2_angle_x(segment.basis));
	    if ((angle > M_PI / 3 && angle < 2 * (M_PI / 3)) ||
		(angle < -M_PI / 3 && angle > -2 * (M_PI / 3)))
	    {
		data->sight.final_point = *(data->passivebase);
		data->sight.final_point.x += actor->facing * -10.0;
		actor->speed.x   = 0.0;
		data->steplength = 0.0;
	    }
	    else data->sight.final_point = isp;
	}
	else data->sight.final_point = isp;

	data->steplength = v2_length(v2_sub(data->sight.final_point, *(data->activebase)));
    }
    else if (data->activebase == NULL)
    {
	/* set bases if needed */

	data->activebase  = &(actor->points.base_a);
	data->passivebase = &(actor->points.base_b);
    }
}

/* do actual step */

void actor_modifier_walk_dostep(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_walk_t* data  = modifier->data;

    if (fabs(actor->speed.x) > 0.001)
    {
	/* get stepvector */

	v2_t  stepvector       = v2_sub(data->sight.final_point, *(data->activebase));
	float stepvectorlength = v2_length(stepvector);

	/* move active base, avoid null stepvector */

	if (stepvectorlength > 0.001)
	{
	    stepvector          = v2_resize(stepvector, fabs(actor->speed.x) * args->delta);
	    *(data->activebase) = v2_add(*(data->activebase), stepvector);
	}

	/* check final step */

	if (stepvectorlength - fabs(actor->speed.x) * args->delta < fabs(actor->speed.x) * args->delta)
	{
	    /* force foot change */

	    data->is_moving = 0;

	    /* check if we have to fall */

	    actor_modifier_walk_check_ground(modifier, args);
	}

	/* if leg is stopping, also check for no ground */

	if (fabs(actor->speed.x) < 0.1) actor_modifier_walk_check_ground(modifier, args);
    }
}

/* new modifier state */

void actor_modifier_walk_new(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_walk_t* data  = modifier->data;

    if (args->type == NULL)
    {
	/* ignore non walling states */

	if (actor->state == kActorStateWalk)
	{
	    if (args->control_state.jump_pressed == 1) data->vertical_direction = 1;
	    if (args->control_state.squat_pressed == 1) data->vertical_direction = 0;

	    /* update base positions */

	    actor_modifier_walk_speed(modifier, args);
	    actor_modifier_walk_newtarget(modifier, args);
	    actor_modifier_walk_dostep(modifier, args);

	    /* update other points */

	    actor_modifier_walk_points_new_hip(modifier, args);
	    actor_modifier_walk_points_new_feet(modifier, args);
	    actor_modifier_walk_points_new_hands(modifier, args);

	    /* do jump if bases are in position */

	    if (args->control_state.jump_pressed == 1 && actor->control_state.jump_pressed == 0)
	    {
		if (actor->speed.x < 0.1)
		{
		    actor->speed.x = actor->facing == kActorFacingRight ? actor->metrics.runspeed : -actor->metrics.runspeed;
		}
		data->wants_to_jump = 1;
	    }

	    if (args->control_state.squat_pressed == 1 && actor->control_state.squat_pressed == 0)
	    {
		cmdqueue_add(args->cmdqueue, "scene.pickup", actor, NULL);
	    }

	    if (args->control_state.shoot_pressed == 1 && actor->control_state.shoot_pressed == 0)
	    {
		attack_t* attack = attack_alloc(actor, actor->points.neck, v2_sub(actor->points.hand_a, actor->points.neck), 1000.0);
		cmdqueue_add(args->cmdqueue, "scene.shoot", NULL, attack);
		REL(attack);
	    }

	    if (data->is_moving == 0 && data->wants_to_jump == 1)
	    {
		data->wants_to_jump = 0;
		actor->state        = kActorStateJump;
	    }
	}
	else data->is_moving = 0;
    }
}

#endif
