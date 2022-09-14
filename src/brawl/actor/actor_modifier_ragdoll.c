
#ifndef actor_modifier_ragdoll_h
#define actor_modifier_ragdoll_h

#include "actor_modifier_types.c"
#include "math1.c"
#include "physics2.c"
#include "zc_memory.c"
#include "zc_util2.c"
#include <stdio.h>

struct actor_modifier_ragdoll_masses
{
    mass2_t* head;
    mass2_t* neck;
    mass2_t* hip;
    mass2_t* hand_a;
    mass2_t* hand_b;
    mass2_t* elbow_a;
    mass2_t* elbow_b;
    mass2_t* knee_a;
    mass2_t* knee_b;
    mass2_t* ankle_a;
    mass2_t* ankle_b;
};

struct actor_modifier_ragdoll_dguards
{
    dguard2_t* headtoneck;
    dguard2_t* necktohip;
    dguard2_t* necktoelbow_a;
    dguard2_t* necktoelbow_b;
    dguard2_t* elbowtohand_a;
    dguard2_t* elbowtohand_b;
    dguard2_t* hiptoknee_a;
    dguard2_t* hiptoknee_b;
    dguard2_t* kneetoankle_a;
    dguard2_t* kneetoankle_b;
};

struct actor_modifier_ragdoll_aguards
{
    aguard2_t* neck;
    aguard2_t* hip_a;
    aguard2_t* hip_b;
    aguard2_t* knee_a;
    aguard2_t* knee_b;
    aguard2_t* elbow_a;
    aguard2_t* elbow_b;
};

typedef struct _actor_modifier_ragdoll_t actor_modifier_ragdoll_t;
struct _actor_modifier_ragdoll_t
{
    char hit_inited;
    char rag_inited;
    char is_dragged;

    float ratio_a;
    float ratio_b;
    float bendsize;

    attack_t attack;
    v2_t     hitpoint;

    mass2_t* hitmass_a;
    mass2_t* hitmass_b;

    vec_t*      masses;
    vec_t*      aguards;
    vec_t*      dguards;
    surfaces_t* emptysurfaces;

    struct actor_modifier_ragdoll_masses  mas;
    struct actor_modifier_ragdoll_aguards ang;
    struct actor_modifier_ragdoll_dguards dis;
};

actor_modifier_t* actor_modifier_ragdoll_alloc(void);
void              actor_modifier_ragdoll_new(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_ragdoll_destruct(void* pointer);
void              actor_modifier_ragdoll_resetkeepers(actor_modifier_t* modifier, actor_modifier_args_t* args);
char              actor_modifier_ragdoll_hit(actor_modifier_t* modifier, actor_modifier_args_t* args, attack_t* attack, char bone, v2_t point);
void              actor_modifier_ragdoll_do_hit(actor_modifier_t* modifier, actor_modifier_args_t* args);
void              actor_modifier_ragdoll_setdragged(actor_modifier_t* modifier, char dragged);

#endif /* actor_modifier_ragdoll_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor.c"
#include "actor_modifier_ragdoll.c"
#include <string.h>

/* default state */

actor_modifier_t* actor_modifier_ragdoll_alloc()
{

    struct actor_modifier_ragdoll_masses masses = {
	.head    = mass2_alloc(v2_init(0.0, 0.0), 4.0, 5.0, 0.4),
	.neck    = mass2_alloc(v2_init(0.0, 0.0), 4.0, 10.0, 0.4),
	.hip     = mass2_alloc(v2_init(0.0, 0.0), 4.0, 15.0, 0.4),
	.hand_a  = mass2_alloc(v2_init(0.0, 0.0), 4.0, 5.0, 0.4),
	.hand_b  = mass2_alloc(v2_init(0.0, 0.0), 4.0, 5.0, 0.4),
	.elbow_a = mass2_alloc(v2_init(0.0, 0.0), 4.0, 9.0, 0.4),
	.elbow_b = mass2_alloc(v2_init(0.0, 0.0), 4.0, 9.0, 0.4),
	.knee_a  = mass2_alloc(v2_init(0.0, 0.0), 4.0, 8.0, 0.4),
	.knee_b  = mass2_alloc(v2_init(0.0, 0.0), 4.0, 8.0, 0.4),
	.ankle_a = mass2_alloc(v2_init(0.0, 0.0), 4.0, 5.0, 0.4),
	.ankle_b = mass2_alloc(v2_init(0.0, 0.0), 4.0, 5.0, 0.4),
    };

    struct actor_modifier_ragdoll_dguards dguards = {
	.headtoneck    = dguard2_alloc(masses.head, masses.neck, 0.0, 0.0),
	.necktohip     = dguard2_alloc(masses.neck, masses.hip, 0.0, 0.0),
	.necktoelbow_a = dguard2_alloc(masses.neck, masses.elbow_a, 0.0, 0.0),
	.necktoelbow_b = dguard2_alloc(masses.neck, masses.elbow_b, 0.0, 0.0),
	.elbowtohand_a = dguard2_alloc(masses.elbow_a, masses.hand_a, 0.0, 0.0),
	.elbowtohand_b = dguard2_alloc(masses.elbow_b, masses.hand_b, 0.0, 0.0),
	.hiptoknee_a   = dguard2_alloc(masses.hip, masses.knee_a, 0.0, 0.0),
	.hiptoknee_b   = dguard2_alloc(masses.hip, masses.knee_b, 0.0, 0.0),
	.kneetoankle_a = dguard2_alloc(masses.knee_a, masses.ankle_a, 0.0, 0.0),
	.kneetoankle_b = dguard2_alloc(masses.knee_b, masses.ankle_b, 0.0, 0.0)};

    struct actor_modifier_ragdoll_aguards aguards = {
	.neck    = aguard2_alloc(masses.head, masses.neck, masses.hip, M_PI - M_PI / 4, M_PI + M_PI / 4),
	.hip_a   = aguard2_alloc(masses.neck, masses.hip, masses.knee_a, M_PI, 2 * M_PI - M_PI / 8),
	.hip_b   = aguard2_alloc(masses.neck, masses.hip, masses.knee_b, M_PI, 2 * M_PI - M_PI / 8),
	.knee_a  = aguard2_alloc(masses.hip, masses.knee_a, masses.ankle_a, M_PI / 8, M_PI),
	.knee_b  = aguard2_alloc(masses.hip, masses.knee_b, masses.ankle_b, M_PI / 8, M_PI),
	.elbow_a = aguard2_alloc(masses.neck, masses.elbow_a, masses.hand_a, M_PI, 2 * M_PI - M_PI / 8),
	.elbow_b = aguard2_alloc(masses.neck, masses.elbow_b, masses.hand_b, M_PI, 2 * M_PI - M_PI / 8)};

    actor_modifier_t*         modifier = CAL(sizeof(actor_modifier_t), actor_modifier_ragdoll_destruct, NULL);
    actor_modifier_ragdoll_t* data     = CAL(sizeof(actor_modifier_ragdoll_t), NULL, NULL);

    modifier->data = data;
    modifier->_new = actor_modifier_ragdoll_new;

    data->masses        = VNEW();
    data->aguards       = VNEW();
    data->dguards       = VNEW();
    data->emptysurfaces = surfaces_alloc(0, 100.0);

    data->hit_inited = 0;
    data->rag_inited = 0;

    data->ratio_a  = 0;
    data->ratio_b  = 0;
    data->bendsize = 0.0;

    data->hitmass_a = NULL;
    data->hitmass_b = NULL;
    data->hitpoint  = v2_init(0.0, 0.0);

    data->attack.power = 0.0;
    data->attack.actor = NULL;
    data->attack.trans = v2_init(0.0, 0.0);
    data->attack.basis = v2_init(0.0, 0.0);

    VADD(data->masses, masses.head);
    VADD(data->masses, masses.neck);
    VADD(data->masses, masses.hip);
    VADD(data->masses, masses.elbow_a);
    VADD(data->masses, masses.elbow_b);
    VADD(data->masses, masses.hand_a);
    VADD(data->masses, masses.hand_b);
    VADD(data->masses, masses.knee_b);
    VADD(data->masses, masses.knee_a);
    VADD(data->masses, masses.ankle_b);
    VADD(data->masses, masses.ankle_a);

    VADD(data->aguards, aguards.neck);
    VADD(data->aguards, aguards.hip_a);
    VADD(data->aguards, aguards.hip_b);
    VADD(data->aguards, aguards.knee_a);
    VADD(data->aguards, aguards.knee_b);
    VADD(data->aguards, aguards.elbow_a);
    VADD(data->aguards, aguards.elbow_b);

    VADD(data->dguards, dguards.headtoneck);
    VADD(data->dguards, dguards.necktohip);
    VADD(data->dguards, dguards.necktoelbow_a);
    VADD(data->dguards, dguards.necktoelbow_b);
    VADD(data->dguards, dguards.elbowtohand_a);
    VADD(data->dguards, dguards.elbowtohand_b);
    VADD(data->dguards, dguards.hiptoknee_a);
    VADD(data->dguards, dguards.hiptoknee_b);
    VADD(data->dguards, dguards.kneetoankle_a);
    VADD(data->dguards, dguards.kneetoankle_b);

    data->mas = masses;
    data->ang = aguards;
    data->dis = dguards;

    return modifier;
}

/* cleanup */

void actor_modifier_ragdoll_destruct(void* pointer)
{
    actor_modifier_t*         modifier = pointer;
    actor_modifier_ragdoll_t* data     = modifier->data;

    for (int index = 0; index < data->masses->length; index++) REL(data->masses->data[index]);
    for (int index = 0; index < data->aguards->length; index++) REL(data->aguards->data[index]);
    for (int index = 0; index < data->dguards->length; index++) REL(data->dguards->data[index]);

    REL(data->masses);
    REL(data->aguards);
    REL(data->dguards);
    REL(data->emptysurfaces);
    REL(modifier->data);
}

/* resets state */

void actor_modifier_ragdoll_reset(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_modifier_ragdoll_t* data = modifier->data;
    data->rag_inited               = 0;
}

/* get point positions from actor */

void actor_modifier_ragdoll_get(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    data->mas.head->trans    = actor->points.head;
    data->mas.neck->trans    = actor->points.neck;
    data->mas.hip->trans     = actor->points.hip;
    data->mas.knee_a->trans  = actor->points.knee_a;
    data->mas.knee_b->trans  = actor->points.knee_b;
    data->mas.ankle_a->trans = actor->points.ankle_a;
    data->mas.ankle_b->trans = actor->points.ankle_b;
    data->mas.elbow_a->trans = actor->points.elbow_a;
    data->mas.elbow_b->trans = actor->points.elbow_b;
    data->mas.hand_a->trans  = actor->points.hand_a;
    data->mas.hand_b->trans  = actor->points.hand_b;
}

/* set actor point positions */

void actor_modifier_ragdoll_set(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    actor->points.head    = data->mas.head->trans;
    actor->points.neck    = data->mas.neck->trans;
    actor->points.hip     = data->mas.hip->trans;
    actor->points.knee_a  = data->mas.knee_a->trans;
    actor->points.knee_b  = data->mas.knee_b->trans;
    actor->points.ankle_a = data->mas.ankle_a->trans;
    actor->points.ankle_b = data->mas.ankle_b->trans;
    actor->points.elbow_a = data->mas.elbow_a->trans;
    actor->points.elbow_b = data->mas.elbow_b->trans;
    actor->points.hand_a  = data->mas.hand_a->trans;
    actor->points.hand_b  = data->mas.hand_b->trans;
}

/* fix dguard distances */

void actor_modifier_ragdoll_resetkeepers(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    if (actor->facing == kActorFacingRight)
    {
	data->ang.neck->angle_min    = M_PI - M_PI / 4;
	data->ang.neck->angle_max    = M_PI + M_PI / 4;
	data->ang.hip_a->angle_min   = M_PI;
	data->ang.hip_a->angle_max   = M_PI * 2.0 - M_PI / 8;
	data->ang.hip_b->angle_min   = M_PI;
	data->ang.hip_b->angle_max   = M_PI * 2.0 - M_PI / 8;
	data->ang.knee_a->angle_min  = M_PI / 8;
	data->ang.knee_a->angle_max  = M_PI;
	data->ang.knee_b->angle_min  = M_PI / 8;
	data->ang.knee_b->angle_max  = M_PI;
	data->ang.elbow_b->angle_min = M_PI;
	data->ang.elbow_b->angle_max = 2 * M_PI - M_PI / 8;
    }
    else
    {
	data->ang.neck->angle_min    = M_PI - M_PI / 4;
	data->ang.neck->angle_max    = M_PI + M_PI / 4;
	data->ang.hip_a->angle_min   = M_PI / 8;
	data->ang.hip_a->angle_max   = M_PI;
	data->ang.hip_b->angle_min   = M_PI / 8;
	data->ang.hip_b->angle_max   = M_PI;
	data->ang.knee_a->angle_min  = M_PI;
	data->ang.knee_a->angle_max  = M_PI * 2.0 - M_PI / 8;
	data->ang.knee_b->angle_min  = M_PI;
	data->ang.knee_b->angle_max  = M_PI * 2.0 - M_PI / 8;
	data->ang.elbow_b->angle_min = 0.0;
	data->ang.elbow_b->angle_max = M_PI - M_PI / 8;
    }

    actor_modifier_ragdoll_get(modifier, args);

    for (int index = 0; index < data->dguards->length; index++) dguard2_resetdistance(data->dguards->data[index]);
}

/* get dguad result type */

struct get_dgurads_result
{
    dguard2_t* guards[10];
};

/* get ik chain from given mass */

struct get_dgurads_result actor_modifier_ragdoll_get_ik_dguards(actor_modifier_t* modifier, actor_modifier_args_t* args, mass2_t* mass)
{
    actor_modifier_ragdoll_t* data   = modifier->data;
    struct get_dgurads_result result = {0};

    if (mass == data->mas.head)
    {
	struct get_dgurads_result result = {.guards = {data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else if (mass == data->mas.neck)
    {
	struct get_dgurads_result result = {.guards = {data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else if (mass == data->mas.hip)
    {
	struct get_dgurads_result result = {.guards = {data->dis.necktohip, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b, data->dis.headtoneck, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b}};
	return result;
    }
    else if (mass == data->mas.knee_a)
    {
	struct get_dgurads_result result = {.guards = {data->dis.hiptoknee_a, data->dis.kneetoankle_a, data->dis.necktohip, data->dis.hiptoknee_b, data->dis.kneetoankle_b, data->dis.headtoneck, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b}};
	return result;
    }
    else if (mass == data->mas.knee_b)
    {
	struct get_dgurads_result result = {.guards = {data->dis.hiptoknee_b, data->dis.kneetoankle_b, data->dis.necktohip, data->dis.hiptoknee_a, data->dis.kneetoankle_a, data->dis.headtoneck, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b}};
	return result;
    }
    else if (mass == data->mas.ankle_a)
    {
	struct get_dgurads_result result = {.guards = {data->dis.kneetoankle_a, data->dis.hiptoknee_a, data->dis.necktohip, data->dis.hiptoknee_b, data->dis.kneetoankle_b, data->dis.headtoneck, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b}};
	return result;
    }
    else if (mass == data->mas.ankle_b)
    {
	struct get_dgurads_result result = {.guards = {data->dis.kneetoankle_b, data->dis.hiptoknee_b, data->dis.necktohip, data->dis.hiptoknee_a, data->dis.kneetoankle_a, data->dis.headtoneck, data->dis.necktoelbow_a, data->dis.necktoelbow_b, data->dis.elbowtohand_a, data->dis.elbowtohand_b}};
	return result;
    }
    else if (mass == data->mas.elbow_a)
    {
	struct get_dgurads_result result = {.guards = {data->dis.necktoelbow_a, data->dis.elbowtohand_a, data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_b, data->dis.elbowtohand_b, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else if (mass == data->mas.elbow_b)
    {
	struct get_dgurads_result result = {.guards = {data->dis.necktoelbow_b, data->dis.elbowtohand_b, data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_a, data->dis.elbowtohand_a, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else if (mass == data->mas.hand_a)
    {
	struct get_dgurads_result result = {.guards = {data->dis.elbowtohand_a, data->dis.necktoelbow_a, data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_b, data->dis.elbowtohand_b, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else if (mass == data->mas.hand_b)
    {
	struct get_dgurads_result result = {.guards = {data->dis.elbowtohand_b, data->dis.necktoelbow_b, data->dis.headtoneck, data->dis.necktohip, data->dis.necktoelbow_a, data->dis.elbowtohand_a, data->dis.hiptoknee_a, data->dis.hiptoknee_b, data->dis.kneetoankle_a, data->dis.kneetoankle_b}};
	return result;
    }
    else return result;
}

/* hit ragdoll */

char actor_modifier_ragdoll_hit(actor_modifier_t* modifier, actor_modifier_args_t* args, attack_t* attack, char bone, v2_t point)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    /* return if not applicable */

    if (data->hit_inited == 1) return 0;
    if (data->rag_inited == 1) return 0;
    if (actor->state == kActorStateDeath) return 0;

    /* get hit masses, ratios and bendsize */

    data->attack   = *attack;
    data->bendsize = 0.0;
    data->hitpoint = point;

    data->hitmass_a = NULL;
    data->hitmass_b = NULL;

    float power_a = 0.0;
    float power_b = 0.0;

    switch (bone)
    {
	case kActorBoneHead:
	    data->hitmass_a = data->mas.head;
	    data->hitmass_b = data->mas.neck;
	    power_a         = 0.9;
	    power_b         = 0.7;
	    break;

	case kActorBoneBody:
	    data->hitmass_a = data->mas.neck;
	    data->hitmass_b = data->mas.hip;
	    power_a         = 0.9;
	    power_b         = 0.5;
	    break;

	case kActorBoneLegUpperA:
	    data->hitmass_a = data->mas.hip;
	    data->hitmass_b = data->mas.knee_a;
	    power_a         = 0.5;
	    power_b         = 0.4;
	    break;

	case kActorBoneLegUpperB:
	    data->hitmass_a = data->mas.hip;
	    data->hitmass_b = data->mas.knee_b;
	    power_a         = 0.5;
	    power_b         = 0.4;
	    break;

	case kActorBoneLegLowerA:
	    data->hitmass_a = data->mas.knee_a;
	    data->hitmass_b = data->mas.ankle_a;
	    power_a         = 0.4;
	    power_b         = 0.3;
	    break;

	case kActorBoneLegLowerB:
	    data->hitmass_a = data->mas.knee_a;
	    data->hitmass_b = data->mas.ankle_b;
	    power_a         = 0.4;
	    power_b         = 0.3;
	    break;
    }

    char blocking = 0;

    if (data->hitmass_a != NULL)
    {
	actor_modifier_ragdoll_resetkeepers(modifier, args);

	float length   = v2_length(v2_sub(data->hitmass_a->trans, data->hitmass_b->trans));
	float length_a = v2_length(v2_sub(data->hitpoint, data->hitmass_a->trans));
	float length_b = v2_length(v2_sub(data->hitpoint, data->hitmass_b->trans));

	data->ratio_a = length_b / length;
	data->ratio_b = length_a / length;

	/* check blocking */

	if (actor->control_state.block_pressed == 1 && actor->facing == kActorFacingRight && data->attack.basis.x < 0.0) blocking = 1;
	if (actor->control_state.block_pressed == 1 && actor->facing == kActorFacingLeft && data->attack.basis.x > 0.0) blocking = 1;

	/* alter speed */

	float plusspeed = data->attack.power / 5.0;
	if (data->attack.basis.x <= 0.0) plusspeed *= -1;
	actor->speed.x += plusspeed;
	if (actor->speed.x > actor->metrics.runspeed) actor->speed.x = actor->metrics.runspeed;
	if (actor->speed.x < -actor->metrics.runspeed) actor->speed.x = -actor->metrics.runspeed;

	if (blocking == 1) data->attack.power *= .1;

	/* reset mass basises */

	for (int index = 0; index < data->masses->length; index++)
	{
	    mass2_t* mass = data->masses->data[index];
	    mass->basis   = v2_init(0.0, 0.0);
	}

	data->bendsize = data->attack.power * 2.0;

	if (actor->state != kActorStateKick && blocking == 0)
	{
	    data->hit_inited = 1;
	    actor->gothit    = 1;
	}

	actor->health -= power_a * data->attack.power;

	/* check death */

	if (actor->health < 0.0)
	{
	    printf("DEATH BY LOW HEALTH %f\n", actor->health);
	    actor->state     = kActorStateDeath;
	    data->hit_inited = 0;
	    actor->gothit    = 0;
	    if (strcmp(actor->name, "hero") == 0)
	    {
		str_t* text = str_frombytes("Health problems");
		cmdqueue_delay(args->cmdqueue, "scene.showwasted", text, NULL, args->ticks + 180);
		REL(text);
	    }
	}
    }

    /* hit is success when it was not blocked */

    if (blocking == 0 && actor->state == kActorStateKick)
    {
	actor->state = kActorStateWalk;
    }

    return blocking == 0;
}

/* do hit movement */

void actor_modifier_ragdoll_do_hit(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    if (data->hit_inited == 1)
    {
	/* reduce bend size */

	data->bendsize -= (data->attack.power / 10.0) * args->delta;

	if (data->bendsize > 0.0)
	{
	    actor_modifier_ragdoll_get(modifier, args);

	    /* calculate positions */

	    v2_t hit = v2_resize(data->attack.basis, data->bendsize / 2.0);

	    data->hitmass_a->basis = v2_scale(hit, data->ratio_a);
	    data->hitmass_b->basis = v2_scale(hit, data->ratio_b);

	    /* resolve distances inverse kinematically */

	    struct get_dgurads_result ikguards = actor_modifier_ragdoll_get_ik_dguards(modifier, args, data->hitmass_a);

	    for (int dguardindex = 0; dguardindex < 10; dguardindex++) dguard2_new(ikguards.guards[dguardindex], args->delta);

	    /* move */

	    for (int index = 0; index < data->masses->length; index++)
	    {
		mass2_t* mass = data->masses->data[index];
		mass->trans   = v2_add(mass->trans, mass->basis);
		mass->basis   = v2_init(0.0, 0.0);
	    }

	    actor_modifier_ragdoll_set(modifier, args);
	}
	else
	{
	    /* reset hit animation */

	    data->hit_inited = 0;
	    actor->gothit    = 0;
	}
    }
}

/* inits rag movement */

void actor_modifier_ragdoll_initrag(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    /* lift ankles so they wont go under surface */

    actor->points.ankle_a.y += 20.0;
    actor->points.ankle_b.y += 20.0;

    /* reset angle and distance keepers */

    actor_modifier_ragdoll_resetkeepers(modifier, args);

    /* add walk speed to masses */

    for (int index = 0; index < data->masses->length; index++)
    {
	mass2_t* mass = data->masses->data[index];
	mass->basis.x = actor->speed.x / 5.0;
	mass->basis.y = actor->speed.y;
    }

    /* get hit masspoints, add hit vector */

    if (data->hitmass_a != NULL)
    {
	data->hitmass_a->basis = v2_add(data->hitmass_a->basis, v2_resize(data->attack.basis, 10.0 * data->ratio_a));
	data->hitmass_b->basis = v2_add(data->hitmass_b->basis, v2_resize(data->attack.basis, 10.0 * data->ratio_b));
    }

    /* set inited state */

    data->rag_inited = 1;
}

/* do ragdoll movement */

void actor_modifier_ragdoll_do_ragdoll(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t*                  actor = args->actor;
    actor_modifier_ragdoll_t* data  = modifier->data;

    /* init rag if needed */

    if (data->rag_inited == 0) actor_modifier_ragdoll_initrag(modifier, args);

    if (data->rag_inited == 1)
    {
	/* send punch action if flying */

	if (fabs(data->mas.ankle_a->basis.x) > 4.0)
	{
	    attack_t* attack = attack_alloc(actor, actor->points.neck, v2_sub(actor->points.hip, actor->points.neck), actor->metrics.hitpower);
	    cmdqueue_add(args->cmdqueue, "scene.punch", NULL, attack);
	    REL(attack);
	}

	/* get actual point positions */

	actor_modifier_ragdoll_get(modifier, args);

	/* add gravity to masses */

	physics2_set_gravity(data->masses, args->gravity, args->delta);

	/* add distance guard forces to masses */

	physics2_set_distances(data->dguards, args->delta);

	/* add angle guard forces to masses */

	physics2_set_angles(data->aguards);

	/* move mass points to endpoint with collision */

	if (data->is_dragged == 1) physics2_set_positions(data->masses, data->emptysurfaces, args->delta);
	else physics2_set_positions(data->masses, args->surfaces, args->delta);

	/* set actor point positions based on rag mass positions */

	actor_modifier_ragdoll_set(modifier, args);

	/* remove actor if fallen too deep */

	if (actor->points.ankle_a.y < -10000.0 || actor->points.ankle_b.y < -10000.0)
	{
	    cmdqueue_add(args->cmdqueue, "scene.removeactor", NULL, actor);
	}
    }
}

/* new modifier state */

void actor_modifier_ragdoll_new(actor_modifier_t* modifier, actor_modifier_args_t* args)
{
    actor_t* actor = args->actor;

    if (args->type == NULL)
    {
	if (actor->state != kActorStateDeath) actor_modifier_ragdoll_do_hit(modifier, args);
	else actor_modifier_ragdoll_do_ragdoll(modifier, args);
    }
    else if (strcmp(args->type, "reset") == 0)
    {
	actor_modifier_ragdoll_reset(modifier, args);
    }
}

/* sets dragged state */

void actor_modifier_ragdoll_setdragged(actor_modifier_t* modifier, char dragged)
{
    actor_modifier_ragdoll_t* data = modifier->data;
    data->is_dragged               = dragged;
}

#endif
