
#ifndef actor_h
#define actor_h

#include "actor_modifier_types.c"
#include "base_metrics.c"
#include "ogl.c"
#include "settings.c"
#include <stdio.h>

#define kActorStateJump 0
#define kActorStateWalk 1
#define kActorStateKick 2
#define kActorStateHit 3
#define kActorStateDeath 4

#define kActorFacingLeft -1
#define kActorFacingRight 1

#define kActorBoneNone 0
#define kActorBoneHead 1
#define kActorBoneBody 2
#define kActorBoneLegUpperA 3
#define kActorBoneLegUpperB 4
#define kActorBoneLegLowerA 5
#define kActorBoneLegLowerB 6

#define kParabolaMaxLevel 10

typedef struct _actor_t actor_t;
struct _actor_t
{
    char* name;

    int8_t state;
    int8_t gothit;
    int8_t facing;

    float power;
    float health;

    float power_rate;
    float health_rate;

    v2_t speed;

    mtvec_t* modifiers;

    actor_modifier_t* ik;
    actor_modifier_t* walk;
    actor_modifier_t* jump;
    actor_modifier_t* kick;

    actor_points_t points;
    base_metrics_t base_metrics;

    actor_metrics_t metrics;
    control_state_t control_state;
};

actor_t* actor_alloc(v2_t position, char* name);
void     actor_levelup(actor_t* actor, uint8_t levels);
void     actor_dealloc(void* pointer);
void     actor_reset(actor_t* actor, v2_t position, actor_modifier_args_t* args);
void     actor_new(actor_t* actor, actor_modifier_args_t* args);
char     actor_hit(actor_t* actor, attack_t* attack, v2_t* isp);
void     actor_loadstate(actor_t* actor);
void     actor_savestate(actor_t* actor);
void     actor_updatemetrics(actor_t* actor, base_metrics_t metrics);

#endif /* actor_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor.c"
#include "actor_modifier_jump.c"
#include "actor_modifier_kick.c"
#include "actor_modifier_ragdoll.c"
#include "actor_modifier_walk.c"
#include "mtcstr.c"
#include "mtstr.c"
#include <float.h>

/* default state */

actor_t* actor_alloc(v2_t position, char* name)
{
    actor_t* actor = mtmem_calloc(sizeof(actor_t), actor_dealloc);

    actor->name = mtcstr_fromcstring(name);

    actor->state  = kActorStateJump;
    actor->gothit = 0;
    actor->facing = 1;

    actor->power  = 0.0;
    actor->health = 0.0;

    actor->power_rate  = 0.09;
    actor->health_rate = 0.02;

    actor->modifiers = mtvec_alloc();

    actor->ik   = actor_modifier_ragdoll_alloc();
    actor->walk = actor_modifier_walk_alloc();
    actor->jump = actor_modifier_jump_alloc();
    actor->kick = actor_modifier_kick_alloc();

    actor->points.base_a = position;
    actor->points.base_b = position;

    mtvec_add(actor->modifiers, actor->jump);
    mtvec_add(actor->modifiers, actor->walk);
    mtvec_add(actor->modifiers, actor->ik);
    mtvec_add(actor->modifiers, actor->kick);

    mtmem_release(actor->ik);
    mtmem_release(actor->walk);
    mtmem_release(actor->jump);
    mtmem_release(actor->kick);

    actor_updatemetrics(actor, base_metrics_default());

    return actor;
}

/* cleanup */

void actor_dealloc(void* pointer)
{
    actor_t* actor = pointer;
    mtmem_release(actor->name);
    mtmem_release(actor->modifiers);
}

/* resets actor to default state and wanted positon */

void actor_reset(actor_t* actor, v2_t position, actor_modifier_args_t* args)
{
    v2_t            nullvector    = {0};
    control_state_t control_state = {0};

    actor->state  = kActorStateJump;
    actor->gothit = 0;
    actor->facing = 1;

    actor->power  = actor->metrics.maxpower;
    actor->health = actor->metrics.maxhealth;

    actor->power_rate  = 0.5;
    actor->health_rate = 0.02;

    actor->speed = nullvector;

    actor->points.base_a = position;
    actor->points.base_b = position;

    actor->control_state = control_state;

    for (int index = 0; index < actor->modifiers->length; index++)
    {
	actor_modifier_t* modifier = actor->modifiers->data[index];
	modifier->_new(modifier, args);
    }
}

/* levels actor up with given levels */

void actor_levelup(actor_t* actor, uint8_t levels)
{
    uint8_t maxlevels = kParabolaMaxLevel - actor->metrics.level;
    if (levels > maxlevels) levels = maxlevels;
    for (int index = 0; index < levels; index++)
    {
	actor->metrics.maxhealth *= 1.1;
	actor->metrics.maxpower *= 1.05;
	actor->metrics.level += 1;

	actor->health_rate *= 1.1;
	actor->power_rate *= 1.05;
    }
}

/* new actor state */

void actor_new(actor_t* actor, actor_modifier_args_t* args)
{
    if (actor->state != kActorStateDeath)
    {
	if (actor->name[0] == 'h')
	{
	    if (actor->power < actor->metrics.maxpower) actor->power += actor->power_rate * args->delta * 2.0;
	    else actor->power = actor->metrics.maxpower;

	    if (actor->health < actor->metrics.maxhealth) actor->health += actor->health_rate * args->delta * 2.0;
	    else actor->health = actor->metrics.maxhealth;
	}
	else
	{
	    if (actor->power < actor->metrics.maxpower) actor->power += actor->power_rate * args->delta;
	    else actor->power = actor->metrics.maxpower;
	}
    }

    for (int index = 0; index < actor->modifiers->length; index++)
    {
	actor_modifier_t* modifier = actor->modifiers->data[index];
	modifier->_new(modifier, args);
    }

    actor->control_state = args->control_state;
}

/* updates metrics */

void actor_updatemetrics(actor_t* actor, base_metrics_t metrics)
{
    actor->base_metrics  = metrics;
    actor->metrics.color = metrics.color_a;

    if (metrics.hitpower < 0.0) metrics.hitpower = 0.0;
    if (metrics.hitrate < 0.0) metrics.hitrate = 0.0;
    if (metrics.stamina < 0.0) metrics.stamina = 0.0;
    if (metrics.speed < 0.0) metrics.speed = 0.0;
    if (metrics.hitpower > 1.0) metrics.hitpower = 1.0;
    if (metrics.hitrate > 1.0) metrics.hitrate = 1.0;
    if (metrics.stamina > 1.0) metrics.stamina = 1.0;
    if (metrics.speed > 1.0) metrics.speed = 1.0;

    float delta = (2.5 - metrics.height) - (metrics.stamina + metrics.speed + metrics.hitpower + metrics.hitrate);
    metrics.height += delta;
    if (metrics.height > 1.0) metrics.height = 1.0;

    actor->metrics.headlength = 20.0 - 4.0 + metrics.height * 8.0;
    actor->metrics.bodylength = 50.0 + metrics.height * 20.0;
    actor->metrics.armlength  = 50.0 + metrics.height * 20.0;
    actor->metrics.leglength  = 60.0 + metrics.height * 20.0;

    actor->metrics.headwidth = 40.0 - 4.0 + metrics.height * 8.0;
    actor->metrics.neckwidth = 4.0 + metrics.hitpower * 5.0;
    actor->metrics.armwidth  = 4.0 + metrics.hitpower * 7.0;
    actor->metrics.hipwidth  = 6.0 + metrics.stamina * 20.0;
    actor->metrics.legwidth  = 6.0 + metrics.stamina * 5.0;

    actor->metrics.runspeed   = 5.0 + metrics.speed * 4.0 + metrics.height;
    actor->metrics.walkspeed  = actor->metrics.runspeed * .6;
    actor->metrics.punchspeed = 7.0 + metrics.hitrate * 2.0;
    actor->metrics.kickspeed  = 0.2 + metrics.hitrate;

    actor->metrics.maxhealth = 100.0 + 10.0 * metrics.stamina;
    actor->metrics.maxpower  = 100.0 + 10.0 * metrics.hitpower;

    actor->metrics.hitpower  = actor->metrics.maxpower * 0.3 + actor->metrics.maxpower * 0.2 * metrics.hitpower;
    actor->metrics.kickpower = actor->metrics.maxpower * 0.3 + actor->metrics.maxpower * 0.2 * metrics.hitpower;

    actor->health = actor->metrics.maxhealth;
    actor->power  = actor->metrics.maxpower;

    actor->metrics.level = 1;
    actor->metrics.xp    = 0;

    uint8_t r  = metrics.color_a >> 24 % 0xFF;
    uint8_t g  = metrics.color_a >> 16 % 0xFF;
    uint8_t b  = metrics.color_a >> 8 % 0xFF;
    uint8_t dr = r;
    uint8_t dg = g;
    uint8_t db = b;

    if (dr > 0x2F) dr -= 0x2F;
    if (dg > 0x2F) dg -= 0x2F;
    if (db > 0x2F) db -= 0x2F;

    actor->metrics.color_a = r << 24 | g << 16 | b << 8 | 0xFE;
    actor->metrics.color_b = dr << 24 | dg << 16 | db << 8 | 0xFE;

    r  = metrics.color_b >> 24 % 0xFF;
    g  = metrics.color_b >> 16 % 0xFF;
    b  = metrics.color_b >> 8 % 0xFF;
    dr = r;
    dg = g;
    db = b;

    if (dr > 0x2F) dr -= 0x2F;
    if (dg > 0x2F) dg -= 0x2F;
    if (db > 0x2F) db -= 0x2F;

    actor->metrics.color_c = r << 24 | g << 16 | b << 8 | 0xFE;
    actor->metrics.color_d = dr << 24 | dg << 16 | db << 8 | 0xFE;
}

/* get hitpoint */

char actor_hit(actor_t* actor, attack_t* attack, v2_t* isp)
{
    v2_t bone;

    bone = v2_sub(actor->points.neck, actor->points.head);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.head, bone);
    if (isp->x != FLT_MAX) return kActorBoneHead;

    bone = v2_sub(actor->points.hip, actor->points.neck);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.neck, bone);
    if (isp->x != FLT_MAX) return kActorBoneBody;

    bone = v2_sub(actor->points.knee_a, actor->points.hip);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.hip, bone);
    if (isp->x != FLT_MAX) return kActorBoneLegUpperA;

    bone = v2_sub(actor->points.knee_a, actor->points.hip);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.hip, bone);
    if (isp->x != FLT_MAX) return kActorBoneLegUpperB;

    bone = v2_sub(actor->points.ankle_a, actor->points.knee_a);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.knee_a, bone);
    if (isp->x != FLT_MAX) return kActorBoneLegLowerA;

    bone = v2_sub(actor->points.ankle_b, actor->points.knee_b);
    *isp = v2_intersect_vectors(attack->trans, attack->basis, actor->points.knee_b, bone);
    if (isp->x != FLT_MAX) return kActorBoneLegLowerB;

    return kActorBoneNone;
}

/* loads state encoded in type container map */

void actor_loadstate(actor_t* actor)
{

    int actorinited = settings_getint("actorinited");
    if (actorinited == 0) return;

    actor->metrics.xp         = settings_getint("xp");
    actor->metrics.level      = settings_getint("level");
    actor->metrics.headlength = settings_getfloat("headlength");
    actor->metrics.bodylength = settings_getfloat("bodylength");
    actor->metrics.armlength  = settings_getfloat("armlength");
    actor->metrics.leglength  = settings_getfloat("leglength");
    actor->metrics.headwidth  = settings_getfloat("headwidth");
    actor->metrics.neckwidth  = settings_getfloat("neckwidth");
    actor->metrics.armwidth   = settings_getfloat("armwidth");
    actor->metrics.bodywidth  = settings_getfloat("bodywidth");
    actor->metrics.hipwidth   = settings_getfloat("hipwidth");
    actor->metrics.legwidth   = settings_getfloat("legwidth");
    actor->metrics.walkspeed  = settings_getfloat("walkspeed");
    actor->metrics.runspeed   = settings_getfloat("runspeed");
    actor->metrics.punchspeed = settings_getfloat("punchspeed");
    actor->metrics.kickspeed  = settings_getfloat("kickspeed");
    actor->metrics.maxpower   = settings_getfloat("maxpower");
    actor->metrics.maxhealth  = settings_getfloat("maxhealth");
    actor->metrics.hitpower   = settings_getfloat("hitpower");
    actor->metrics.kickpower  = settings_getfloat("kickpower");
    actor->metrics.color      = settings_getunsigned("color");
    actor->metrics.color_a    = settings_getunsigned("color_a");
    actor->metrics.color_b    = settings_getunsigned("color_b");
    actor->metrics.color_c    = settings_getunsigned("color_c");
    actor->metrics.color_d    = settings_getunsigned("color_d");

    actor->health = actor->metrics.maxhealth;
}

/* encodes state in type container map */

void actor_savestate(actor_t* actor)
{

    settings_setint("actorinited", 1);

    settings_setint("xp", actor->metrics.xp);
    settings_setint("level", actor->metrics.level);
    settings_setfloat("headlength", actor->metrics.headlength);
    settings_setfloat("bodylength", actor->metrics.bodylength);
    settings_setfloat("armlength", actor->metrics.armlength);
    settings_setfloat("leglength", actor->metrics.leglength);
    settings_setfloat("headhwidth", actor->metrics.headwidth);
    settings_setfloat("neckwidth", actor->metrics.neckwidth);
    settings_setfloat("armwidth", actor->metrics.armwidth);
    settings_setfloat("bodywidth", actor->metrics.bodywidth);
    settings_setfloat("hipwidth", actor->metrics.hipwidth);
    settings_setfloat("legwidth", actor->metrics.legwidth);
    settings_setfloat("walkspeed", actor->metrics.walkspeed);
    settings_setfloat("runspeed", actor->metrics.runspeed);
    settings_setfloat("punchspeed", actor->metrics.punchspeed);
    settings_setfloat("kickspeed", actor->metrics.kickspeed);
    settings_setfloat("maxpower", actor->metrics.maxpower);
    settings_setfloat("maxhealth", actor->metrics.maxhealth);
    settings_setfloat("hitpower", actor->metrics.hitpower);
    settings_setfloat("kickpower", actor->metrics.kickpower);
    settings_setunsigned("color", actor->metrics.color);
    settings_setunsigned("color_a", actor->metrics.color_a);
    settings_setunsigned("color_b", actor->metrics.color_b);
    settings_setunsigned("color_c", actor->metrics.color_c);
    settings_setunsigned("color_d", actor->metrics.color_d);
}

#endif
