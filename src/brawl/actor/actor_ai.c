#ifndef actor_ai_h
#define actor_ai_h

#include "actor.c"
#include "actor_group.c"
#include "floatbuffer.c"
#include <stdio.h>
#include <stdlib.h>

#define kActorAIModeStand 0
#define kActorAIModeGuard 1
#define kActorAIModeFollow 2
#define kActorAIModeAttack 3

typedef struct _actor_ai_t actor_ai_t;
struct _actor_ai_t
{
    float    health;
    uint8_t  mode;
    uint8_t  phase;
    uint32_t color;
    uint32_t punchtick;

    actor_group_t* locked_boss;
    actor_group_t* locked_enemy;

    control_state_t control_state;
    void (*step)(actor_ai_t*, actor_modifier_args_t*, mtvec_t*);
};

actor_ai_t* actor_ai_alloc(actor_t* actor, uint32_t color, float level);
void        actor_ai_set_boss(actor_ai_t* controller, actor_group_t* boss);
void        actor_ai_set_enemy(actor_ai_t* controller, actor_group_t* enemy);
void        actor_ai_new(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* agroups);
void        actor_ai_debug_lines(actor_ai_t* controller, actor_t* actor, floatbuffer_t* linebuffer);
void        actor_ai_releaselocked(actor_ai_t* ai);

#endif /* actor_ai_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_ai.c"
#include "actor_modifier_walk.c"
#include <string.h>

void actor_ai_dealloc(void* pointer);
void actor_ai_stand(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups);
void actor_ai_attack(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups);
void actor_ai_follow(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups);

/* default state */

actor_ai_t* actor_ai_alloc(actor_t* actor, uint32_t color, float level)
{
    actor_ai_t* controller = mtmem_calloc(sizeof(actor_ai_t), actor_ai_dealloc);

    controller->mode  = kActorAIModeStand;
    controller->color = color;
    controller->step  = actor_ai_stand;
    controller->phase = rand() % 60;

    return controller;
}

/* cleanup */

void actor_ai_dealloc(void* pointer)
{
}

/* standing action */

void actor_ai_stand(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups)
{
    if (ai->locked_enemy != NULL) ai->step = actor_ai_attack;
    actor_t* actor = args->actor;

    if (actor->state != kActorStateDeath)
    {
	/* look for enemy or boss */

	if (args->ticks % 15 == 0)
	{
	    actor_group_t* current_group = NULL;
	    for (int index = 0; index < groups->length; index++)
	    {
		actor_group_t* group = groups->data[index];
		if (group->actor == actor)
		{
		    current_group = group;
		    break;
		}
	    }

	    actor_group_t* locked_boss = NULL;
	    for (int index = 0; index < groups->length; index++)
	    {
		actor_group_t* group = groups->data[index];
		actor_t*       enemy = group->actor;

		/* exclude ourselves and dead bodies */

		if (enemy != actor && enemy->state != kActorStateDeath)
		{
		    v2_t distance = v2_sub(enemy->points.hip, actor->points.hip);

		    /* exclude too distant entities */

		    if (fabs(distance.x) < 200.0 + (float) (rand() % 100) && fabs(distance.y) < 80.0)
		    {
			if (enemy->metrics.color != actor->metrics.color)
			{
			    char* strings[3]           = {"HEY!", "COME HERE!", "GET SOME!!"};
			    char* string               = strings[rand() % 2];
			    current_group->currenttext = mtstr_frombytes(string);

			    actor_ai_set_enemy(ai, group);
			    actor_ai_set_boss(ai, NULL);
			}
			else if (strcmp(enemy->name, "hero") == 0)
			{
			    locked_boss = group;
			}
		    }
		}
	    }

	    if (ai->locked_enemy == NULL && ai->locked_boss == NULL && locked_boss != NULL)
	    {
		actor_ai_set_boss(ai, locked_boss);

		char* strings[3]           = {"HI BOSS", "FINALLY!", "HOWDY!!"};
		char* string               = strings[rand() % 2];
		current_group->currenttext = mtstr_frombytes(string);
	    }

	    if (ai->locked_enemy == NULL && ai->locked_boss != NULL)
	    {
		actor_t* boss     = ai->locked_boss->actor;
		v2_t     distance = v2_sub(boss->points.hip, actor->points.hip);
		if (v2_longside(distance) > 50.0) ai->step = actor_ai_follow;
	    }
	}
    }
}

/* follow boss action */

void actor_ai_follow(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups)
{
    if (ai->locked_enemy != NULL) ai->step = actor_ai_attack;
    actor_t* actor = args->actor;

    if (args->ticks % 15 == 0)
    {
	actor_t* boss  = ai->locked_boss->actor;
	v2_t     point = boss->points.hip;

	actor_modifier_walk_t* selfwalk = (actor_modifier_walk_t*) actor->walk->data;
	selfwalk->vertical_direction    = boss->points.base_a.y > actor->points.base_a.y;

	if (actor->facing == kActorFacingRight) point.x += 50.0;
	else point.x -= 50.0;

	v2_t distance = v2_sub(point, actor->points.hip);

	if (v2_longside(distance) > 500.0 || boss->state == kActorStateDeath)
	{
	    /* loose it if too far */

	    control_state_t nullstate = {0};
	    ai->step                  = actor_ai_stand;
	    ai->control_state         = nullstate;

	    actor_ai_set_boss(ai, NULL);
	}
	else
	{
	    /* stop if close */

	    if (fabs(distance.x) > 100.0 + (float) (rand() % 50)) ai->control_state.run_pressed = 1;
	    else ai->control_state.run_pressed = 0;

	    actor->facing = distance.x > 0 ? 1 : -1;

	    if (distance.x > 50.0 && boss->facing == 1)
	    {
		ai->control_state.right_pressed = 1;
		ai->control_state.left_pressed  = 0;
	    }
	    else if (distance.x < -50.0 && boss->facing == -1)
	    {
		ai->control_state.right_pressed = 0;
		ai->control_state.left_pressed  = 1;
	    }
	    else
	    {
		control_state_t nullstate = {0};
		ai->step                  = actor_ai_stand;
		ai->control_state         = nullstate;
	    }
	}
    }
}

/* follow and attack action
	level 1 - walks after enemy, blocks if power level is too low, hit rate based on level
	level 2 - runs after enemy if needed
	level 3 - always blocks, stops it a little time before hitting
	level 4 - runs to the other side of the enemy after hit, always hits when have power
*/

void actor_ai_attack(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* groups)
{

    if ((args->ticks + ai->phase) % 20 == 0)
    {
	actor_t* actor    = args->actor;
	actor_t* enemy    = ai->locked_enemy->actor;
	v2_t     distance = v2_sub(enemy->points.hip, actor->points.hip);

	if (fabs(distance.x) > 500.0 || fabs(distance.y) > 400.0 || enemy->state == kActorStateDeath)
	{
	    /* loose it if too far */

	    control_state_t nullstate = {0};
	    ai->step                  = actor_ai_stand;
	    ai->control_state         = nullstate;

	    actor_ai_set_enemy(ai, NULL);
	}
	else
	{
	    actor_modifier_walk_t* selfwalk = (actor_modifier_walk_t*) actor->walk->data;
	    selfwalk->vertical_direction    = enemy->points.base_a.y > actor->points.base_a.y;

	    /* stop if close */

	    control_state_t oldstate  = ai->control_state;
	    control_state_t nullstate = {0};
	    ai->control_state         = nullstate;

	    if (actor->metrics.level > 1)
	    {
		// run after hero

		if (fabs(distance.x) > 100.0 + (float) (rand() % 50))
		{
		    ai->control_state.run_pressed = 1;
		}
	    }

	    if (distance.x > 1.5 * actor->metrics.armlength)
	    {
		ai->control_state.right_pressed = 1;

		if (actor->metrics.level > 2 && distance.x < 2.5 * actor->metrics.armlength)
		{
		    ai->control_state.block_pressed = 1;
		}
	    }
	    else if (distance.x < -1.5 * actor->metrics.armlength)
	    {
		ai->control_state.left_pressed = 1;
		if (actor->metrics.level > 2 && distance.x > -2.5 * actor->metrics.armlength)
		{
		    ai->control_state.block_pressed = 1;
		}
	    }
	    else
	    {
		if (actor->metrics.level > 2)
		{
		    if (fabs(distance.x) < actor->metrics.armlength / 1.5)
		    {
			if (distance.x < 0) ai->control_state.right_pressed = 1;
			if (distance.x > 0) ai->control_state.left_pressed = 1;
		    }

		    if (actor->metrics.level > 3)
		    {
			if (oldstate.punch_pressed == 1 || oldstate.kick_pressed == 1)
			{
			    if (distance.x > 0) ai->control_state.right_pressed = 1;
			    if (distance.x < 0) ai->control_state.left_pressed = 1;
			    ai->control_state.run_pressed = 1;
			}
		    }
		}

		if (distance.x > 0.0) actor->facing = kActorFacingRight;
		else actor->facing = kActorFacingLeft;

		if (actor->metrics.level < 3)
		{
		    // block once when health is dropped

		    if (ai->health > actor->health) ai->control_state.block_pressed = 1;
		    ai->health = actor->health;
		}
		else
		{
		    //						if ( ai->punchtick == 0 )
		    //						{
		    //							//ai->control_state.block_pressed = 1;
		    //							if ( actor->power > actor->metrics.hitpower ) ai->punchtick = ( args->ticks + ai->phase ) + ( kParabolaMaxLevel + 1 - actor->metrics.level ) * 5;
		    //						}
		}

		//					if ( ai->punchtick > 0 )
		//					{
		//						if ( args->ticks > ai->punchtick ) ai->punchtick = 0;
		//					}

		if (ai->control_state.block_pressed == 0)
		{
		    // if ( actor->metrics.level > 4 || ( actor->metrics.level < 3 && rand() % 2 == 0 ) )

		    int seed = 6 - actor->metrics.level;
		    if (seed < 2) seed = 2;
		    int pick = rand() % seed;

		    if (pick == 0)
		    {
			ai->control_state.punch_pressed = 1;
		    }
		    else if (pick == 1)
		    {
			ai->control_state.kick_pressed = 1;
		    }
		}
	    }
	}
    }
}

/* new actor controller state */

void actor_ai_new(actor_ai_t* ai, actor_modifier_args_t* args, mtvec_t* agroups)
{
    actor_t* actor = args->actor;

    if (actor->state != kActorStateDeath) ai->step(ai, args, agroups);

    args->control_state = ai->control_state;
    actor_new(args->actor, args);
}

/* releases locked actors */

void actor_ai_releaselocked(actor_ai_t* ai)
{
    mtmem_release(ai->locked_enemy);
    mtmem_release(ai->locked_boss);
    ai->locked_enemy = NULL;
    ai->locked_boss  = NULL;
}

/* set enemy */

void actor_ai_set_enemy(actor_ai_t* ai, actor_group_t* enemy)
{
    mtmem_release(ai->locked_enemy);
    ai->locked_enemy = enemy;
    mtmem_retain(enemy);
}

/* set boss */

void actor_ai_set_boss(actor_ai_t* ai, actor_group_t* boss)
{
    mtmem_release(ai->locked_boss);
    ai->locked_boss = boss;
    mtmem_retain(boss);
}

/* adds point to debug point buffer */

void actor_ai_add_line(floatbuffer_t* buffer, v2_t point_a, v2_t point_b, float coord)
{
    floatbuffer_add(buffer, point_a.x);
    floatbuffer_add(buffer, point_a.y);
    floatbuffer_add(buffer, coord);

    floatbuffer_add(buffer, point_b.x);
    floatbuffer_add(buffer, point_b.y);
    floatbuffer_add(buffer, coord);
}

/* add debug lines and points to line and pointbuffers */

void actor_ai_debug_lines(actor_ai_t* controller, actor_t* actor, floatbuffer_t* linebuffer)
{
    float coords = ogl_color_float_from_rgbauint32(0xFFFFFFFF);

    if (actor->state != kActorStateDeath)
    {
	/* update locked enemy's box */

	if (controller->locked_enemy != NULL)
	{
	    actor_t* enemy = (actor_t*) controller->locked_enemy->actor;
	    actor_ai_add_line(linebuffer, v2_add(enemy->points.hip, v2_init(-100.0, 100.0)), v2_add(enemy->points.hip, v2_init(100.0, 100.0)), coords);
	    actor_ai_add_line(linebuffer, v2_add(enemy->points.hip, v2_init(100.0, 100.0)), v2_add(enemy->points.hip, v2_init(100.0, -100.0)), coords);
	    actor_ai_add_line(linebuffer, v2_add(enemy->points.hip, v2_init(100.0, -100.0)), v2_add(enemy->points.hip, v2_init(-100.0, -100.0)), coords);
	    actor_ai_add_line(linebuffer, v2_add(enemy->points.hip, v2_init(-100.0, -100.0)), v2_add(enemy->points.hip, v2_init(-100.0, 100.0)), coords);
	}
    }
}

#endif
