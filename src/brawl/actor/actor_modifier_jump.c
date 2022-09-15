#ifndef actor_modifier_jump_h
#define actor_modifier_jump_h

#include "actor.c"
#include "actor_modifier_jump_points.c"
#include "tween.c"
#include "zc_memory.c"
#include <stdio.h>
#include <stdlib.h>

typedef struct _actor_modifier_jump_t actor_modifier_jump_t;
struct _actor_modifier_jump_t
{
    char phase;
    char toosteep;

    char is_jumping;
    char a_on_ground;
    char b_on_ground;

    float hipmod;
    float stepsize;
    float squatsize;

    movement_t kick;
    movement_t punch_a;
    movement_t punch_b;

    mass2_t* mass_a;
    mass2_t* mass_b;

    vec_t* masses_a;
    vec_t* masses_b;

    v2_t* activefoot;
    v2_t* passivefoot;
    v2_t  squattarget;
};

actor_modifier_t* actor_modifier_jump_alloc(void);
void              actor_modifier_jump_new(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_jump_destruct(void* pointer);
void              actor_modifier_jump_sendkickaction(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_jump_sendpunchaction(actor_modifier_t* modifier, actor_modifier_args_t* args, v2_t basis);
void              actor_modifier_jump_stoppunch(actor_modifier_t* modifier);
void              actor_modifier_jump_stopkick(actor_modifier_t* modifier);

#endif /* actor_modifier_jump_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_modifier_jump.c"
#include <float.h>
#include <string.h>

/* default state */

actor_modifier_t* actor_modifier_jump_alloc()
{
    actor_modifier_t*      modifier = CAL(sizeof(actor_modifier_t), actor_modifier_jump_destruct, NULL);
    actor_modifier_jump_t* data     = CAL(sizeof(actor_modifier_jump_t), NULL, NULL);

    modifier->data = data;
    modifier->_new = actor_modifier_jump_new;

    data->is_jumping = 0;

    data->a_on_ground = 0;
    data->b_on_ground = 0;

    data->hipmod    = 0.0;
    data->stepsize  = FLT_MAX;
    data->squatsize = 0.0;

    data->masses_a = VNEW();
    data->masses_b = VNEW();

    data->mass_a = mass2_alloc(v2_init(0.0, 0.0), 4.0, 10.0, 0.0);
    data->mass_b = mass2_alloc(v2_init(0.0, 0.0), 4.0, 10.0, 0.0);

    VADD(data->masses_a, data->mass_a);
    VADD(data->masses_b, data->mass_b);

    data->toosteep = 0;

    return modifier;
}

/* cleanup */

void actor_modifier_jump_destruct(void* pointer)
{
    actor_modifier_t*      modifier = pointer;
    actor_modifier_jump_t* data     = modifier->data;

    REL(data->masses_a);
    REL(data->masses_b);

    REL(data->mass_a);
    REL(data->mass_b);

    REL(modifier->data);
}

/* sends punch action to controller */

void actor_modifier_jump_sendpunchaction(actor_modifier_t* modifier, actor_modifier_args_t* args, v2_t basis)
{
    actor_t* actor = args->actor;

    attack_t* attack = attack_alloc(actor, actor->points.neck, basis, actor->metrics.hitpower);

    cmdqueue_add(args->cmdqueue, "scene.punch", NULL, attack);

    REL(attack);
}

/* sends kick action to controller */

void actor_modifier_jump_sendkickaction(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t* actor = args->actor;

    attack_t* attack = attack_alloc(actor, actor->points.hip, v2_sub(actor->points.ankle_a, actor->points.hip), actor->metrics.hitpower);

    cmdqueue_add(args->cmdqueue, "scene.kick", NULL, attack);

    REL(attack);
}

/* stops punch movement */

void actor_modifier_jump_stoppunch(actor_modifier_t* modifier)
{
    actor_modifier_jump_t* jump = modifier->data;

    if (jump->punch_a.speed > 0) jump->punch_a.speed *= -1;
    if (jump->punch_b.speed > 0) jump->punch_b.speed *= -1;
}

/* stops kick movement */

void actor_modifier_jump_stopkick(actor_modifier_t* modifier)
{
    actor_modifier_jump_t* jump = modifier->data;

    if (jump->kick.speed > 0) jump->kick.speed *= -1;
}

/* init jump animation */

void actor_modifier_jump_init(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_jump_t* data  = modifier->data;

    /* get active and passive foot */

    if ((actor->facing == kActorFacingRight && (actor->points.base_b.x > actor->points.base_a.x)) ||
	(actor->facing == kActorFacingLeft && (actor->points.base_b.x < actor->points.base_a.x)))
    {
	data->activefoot  = &actor->points.base_b;
	data->passivefoot = &actor->points.base_a;
    }
    else
    {
	data->activefoot  = &actor->points.base_a;
	data->passivefoot = &actor->points.base_b;
    }

    data->phase       = 0;
    data->is_jumping  = 1;
    data->squattarget = v2_init(data->activefoot->x, data->activefoot->y + 10.0);
}

/* new jumping state*/

void actor_modifier_jump_do_jump(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_jump_t* data  = modifier->data;

    if (data->phase == 0)
    {
	/* first we have to squat before jumping, this is the active foot's target */

	/* move back foot to front foot */

	v2_t  stepvector       = v2_sub(data->squattarget, *data->passivefoot);
	float stepvectorlength = v2_length(stepvector);

	if (actor->speed.x < 0.0 && actor->speed.x > -0.5) actor->speed.x = -0.5;
	if (actor->speed.x > 0.0 && actor->speed.x < 0.5) actor->speed.x = 0.5;

	if (data->stepsize == FLT_MAX) data->stepsize = stepvectorlength;

	if (stepvectorlength > fabs(actor->speed.x) && actor->speed.x != 0.0)
	{
	    /* step and squat a little */

	    if (stepvectorlength < data->stepsize / 3.0) data->hipmod += fabs(actor->speed.x) * 1.4 * args->delta;
	    else data->hipmod = -(data->stepsize - stepvectorlength) / 2.0;

	    stepvector         = v2_resize(stepvector, fabs(actor->speed.x) * args->delta);
	    *data->passivefoot = v2_add(*data->passivefoot, stepvector);
	}
	else
	{
	    /* start jumping phase */

	    data->phase     = 1;
	    data->squatsize = 0.0;

	    /* avoid immediate ground intersection */

	    actor->points.base_a.y += 4.0;
	    actor->points.base_b.y += 4.0;

	    actor->speed.y = fabs(actor->speed.x);

	    data->a_on_ground = 0;
	    data->b_on_ground = 0;

	    data->mass_a->trans = actor->points.base_a;
	    data->mass_b->trans = actor->points.base_b;

	    data->mass_a->basis = v2_init(actor->speed.x / 2.0 + .1, actor->speed.y);
	    data->mass_b->basis = v2_init(actor->speed.x / 2.0 - .1, actor->speed.y);

	    data->stepsize = FLT_MAX;
	}
    }
    else
    {
	/* check collision with ground */

	if (data->a_on_ground == 0)
	{
	    physics2_set_gravity(data->masses_a, args->gravity, args->delta);
	    physics2_set_positions(data->masses_a, args->surfaces, args->delta);

	    if (data->mass_a->basis.x == 0.0 && data->mass_a->basis.y == 0.0)
	    {
		data->a_on_ground = 1;
	    }
	}

	if (data->b_on_ground == 0)
	{
	    physics2_set_gravity(data->masses_b, args->gravity, args->delta);
	    physics2_set_positions(data->masses_b, args->surfaces, args->delta);

	    if (data->mass_b->basis.x == 0.0 && data->mass_b->basis.y == 0.0)
	    {
		data->b_on_ground = 1;
	    }
	}

	/* check leg distance */

	//            if ( data->a_on_ground != data->b_on_ground )
	//            {
	//                float distance = v2_length( v2_sub( data->mass_a->trans , data->mass_b->trans ) );
	//                if ( distance > actor->metrics.leglength * 2.0 )
	//				{
	//					printf( "DEATH BY LEGDISTANCE %f %f" , distance , actor->metrics.leglength );
	//					actor->state = kActorStateDeath;
	//                    if ( strcmp( actor->name , "hero" ) == 0 )
	//                    {
	//                        mtstr_t* text = mtstr_frombytes("Fell of from edge");
	//                        cmdqueue_delay( args->cmdqueue, "scene.showwasted", text, NULL, args->ticks + 180 );
	//                        REL( text );
	//                    }
	//				}
	//            }

	/* check if ground reached */

	if (data->a_on_ground == 1 && data->b_on_ground == 1)
	{
	    data->is_jumping = 0;

	    if (actor->speed.y < -15.0)
	    {
		printf("DEATH BY FALL SPEED %f", actor->speed.y);
		actor->state = kActorStateDeath;

		if (strcmp(actor->name, "hero") == 0)
		{
		    str_t* text = str_frombytes("Crushed to the ground");
		    cmdqueue_delay(args->cmdqueue, "scene.showwasted", text, NULL, args->ticks + 180);
		    REL(text);
		}
	    }
	    else actor->state = kActorStateWalk;

	    v2_t basis = v2_init(0.0, -100.0);

	    /* check slope and rejump */

	    physics2_collision_t collision = {0};
	    physics2_collect_intersecting_and_nearby_surfaces(args->surfaces, data->mass_a->trans, basis, 10.0, &collision);
	    physics2_collect_intersecting_and_nearby_surfaces(args->surfaces, data->mass_b->trans, basis, 10.0, &collision);

	    for (int index = 0; index < collision.count; index++)
	    {

		segment2_t segment = collision.segments[index];

		float angle = fabs(v2_angle_x(segment.basis));
		if (angle >= M_PI_2) angle = M_PI - angle;
		if (angle > M_PI / 3)
		{
		    actor->state     = kActorStateJump;
		    data->is_jumping = 1;

		    data->a_on_ground = 0;
		    data->b_on_ground = 0;

		    data->mass_a->basis.x = actor->facing * -2.0;
		    data->mass_b->basis.x = actor->facing * -2.0;
		    data->mass_a->basis.y = 4.0;
		    data->mass_b->basis.y = 4.0;

		    actor->points.base_a.y += 4.0;
		    actor->points.base_b.y += 4.0;

		    actor->speed.y += 30.0;
		    break;
		}
	    }
	}

	if (actor->points.ankle_a.y < -10000.0 || actor->points.ankle_b.y < -10000.0)
	{
	    printf("DEATH BY DISAPPEARING FROM SCENE %f %f", actor->points.ankle_a.y, actor->points.ankle_b.y);

	    cmdqueue_add(args->cmdqueue, "scene.removeactor", NULL, actor);
	    actor->state = kActorStateDeath;
	    if (strcmp(actor->name, "hero") == 0)
	    {
		str_t* text = str_frombytes("Crushed to the ground");
		cmdqueue_delay(args->cmdqueue, "scene.showwasted", text, NULL, args->ticks + 180);
		REL(text);
	    }
	}

	/* calculate final speed */

	actor->speed.y += args->gravity.y * args->delta;

	/* update pointes */

	actor->points.base_a = data->mass_a->trans;
	actor->points.base_b = data->mass_b->trans;
    }
}

/* new modifier state */

void actor_modifier_jump_new(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*               actor = args->actor;
    actor_modifier_jump_t* data  = modifier->data;

    if (args->type == NULL)
    {
	if (actor->state == kActorStateJump)
	{
	    /* update base positions */

	    if (data->is_jumping == 0) actor_modifier_jump_init(modifier, args);
	    else actor_modifier_jump_do_jump(modifier, args);

	    /* update points */

	    actor_modifier_jump_points_new_feet(modifier, args);
	    actor_modifier_jump_points_new_hip(modifier, args);
	    actor_modifier_jump_points_new_hands(modifier, args);
	}
    }
    else if (strcmp(args->type, "reset") == 0)
    {
	data->is_jumping = 0;

	data->a_on_ground = 0;
	data->b_on_ground = 0;

	data->hipmod    = 0.0;
	data->stepsize  = FLT_MAX;
	data->squatsize = 0.0;

	data->toosteep = 0;
    }
}

#endif
