/*
  scene.c
  massbrawl

  Created by Milan Toth on 2016. 11. 27..
  Copyright © 2016. Milan Toth. All rights reserved.
*/

#ifndef _scene_header
#define _scene_header

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "blood.c"
#include "defaults.c"
#include "gun.c"
#include "pivot.c"
#include "scenerenderer.c"

#include "actor_ai.c"
#include "actor_debug.c"
#include "actor_group.c"
#include "actor_hud_element.c"
#include "actor_modifier_jump.c"
#include "actor_modifier_kick.c"
#include "actor_modifier_ragdoll.c"
#include "actor_modifier_walk.c"
#include "actor_skin.c"

#include "element.h"
#include "floatbuffer.c"
#include "textelement.h"
#include "triangulate.c"
#include "zc_cstring.c"
#include "zc_memory.c"

#define kSceneStateAlive 0
#define kSceneStateDead 1
#define kSceneStateFinished 2

typedef struct _zoom_state_t zoom_state_t;
struct _zoom_state_t
{
    float ratio;
    float target;
};

typedef struct _pan_state_t pan_state_t;
struct _pan_state_t
{
    float x;
    float y;
    float target_x;
    float target_y;
};

typedef struct _scene_t scene_t;
struct _scene_t
{

    char  state;
    float phase;

    int game_clear;
    int game_clear_showed;

    pan_state_t  pan;
    zoom_state_t zoom;

    m4_t matrix;
    v2_t lefttop2world;
    v2_t rightbtm2world;

    vec_t* guns;
    vec_t* tips;
    vec_t* bloods;
    vec_t* actors;
    vec_t* debuglines;

    v2_t        endpoint;
    surfaces_t* surfaces;

    actor_group_t* herogroup;
    actor_group_t* enemygroup;

    floatbuffer_t* vertexbuffer;
    cmdqueue_t*    cmdqueue;

    control_state_t control_state;
};

void scene_init(void);
void scene_free(void);

extern scene_t scene;

#endif

#if __INCLUDE_LEVEL__ == 0

#define NANOSVG_IMPLEMENTATION 1
#include "nanosvg.h"

#include "SDL.h"

scene_t scene;

void scene_onmessage(const char* name, void* data);
void scene_kick(attack_t* attack);
void scene_punch(attack_t* attack);
void scene_shoot(attack_t* attack);
void scene_pickup(actor_t* actor);

/* initializer */

void scene_init()
{

    scene.state             = kSceneStateAlive;
    scene.phase             = 0.0;
    scene.game_clear        = 0;
    scene.game_clear_showed = 0;
    scene.zoom.ratio        = 1.0;
    scene.matrix            = m4_defaultidentity();
    scene.lefttop2world     = v2_init(0.0, 0.0);
    scene.rightbtm2world    = v2_init(defaults.width, defaults.height);

    scene.guns       = VNEW();
    scene.tips       = VNEW();
    scene.bloods     = VNEW();
    scene.actors     = VNEW();
    scene.debuglines = VNEW();

    scene.endpoint = v2_init(0.0, 0.0);
    scene.surfaces = surfaces_alloc(100, 100.0);

    scene.herogroup  = NULL;
    scene.enemygroup = NULL;

    scene.vertexbuffer = floatbuffer_alloc();
    scene.cmdqueue     = cmdqueue_alloc();

    /* init renderer */

    scenerenderer_init(defaults.width, defaults.height);

    scenerenderer_create_buffer(0, GL_TRIANGLES); // background state 1
    scenerenderer_create_buffer(1, GL_TRIANGLES); // background state 2
    scenerenderer_create_buffer(2, GL_TRIANGLES); // background state 3
    scenerenderer_create_buffer(3, GL_TRIANGLES); // foreground
    scenerenderer_create_buffer(4, GL_LINES);     // lines
    scenerenderer_create_buffer(5, GL_POINTS);    // points

    /* subscribe to scene channel */

    bus_subscribe("SCN", scene_onmessage);

    /* init hero */

    actor_t* hero = actor_alloc(v2_init(0.0, 0.0), "hero");

    hero->metrics.maxhealth *= 1.5;
    hero->health = hero->metrics.maxhealth;

    actor_ai_t*   heroai   = actor_ai_alloc(hero, 0xFF0000FF, 0.0);
    actor_skin_t* heroskin = actor_skin_alloc();
    element_t*    hud      = actor_hud_element_alloc("HERO", 0xFF0000FF, defaults.scale, defaults.font);

    scene.herogroup = actor_group_alloc(hero, heroskin, heroai);

    actor_group_sethud(scene.herogroup, hud);

    actor_loadstate(hero);
}

/* cleanup */

void scene_free(void)
{

    REL(scene.herogroup);
    REL(scene.enemygroup);

    scenerenderer_free();

    REL(scene.cmdqueue);
    REL(scene.vertexbuffer);

    REL(scene.tips);
    REL(scene.bloods);
    REL(scene.actors);
    REL(scene.surfaces);
    REL(scene.debuglines);
}

/* reset level */

void scene_reset()
{

    vec_reset(scene.guns);
    vec_reset(scene.bloods);
    vec_reset(scene.actors);
    vec_reset(scene.debuglines);

    surfaces_reset(scene.surfaces);

    actor_t* hero = scene.herogroup->actor;

    hero->power  = hero->metrics.maxpower;
    hero->health = hero->metrics.maxhealth;

    vec_ins(scene.actors, scene.herogroup, 0);
}

/* load level */

void scene_load(int levelindex)
{

    /* start music */

    if (levelindex > 0)
    {

	bus_notify("SND", "PLAYMUSIC", NULL);
    }

    scene_reset();

    /* load svg */

    char num[2] = {0};

    snprintf(num, 2, "%i", levelindex);

    char* paths     = cstr_new_format(PATH_MAX, "%s/level%s.svg", defaults.respath, num, NULL);
    char* namespath = cstr_new_format(PATH_MAX, "%s/names.txt", defaults.respath, num, NULL);

    vec_t* scene_pivots = VNEW();
    vec_t* scene_names  = mtfile_readlines(namespath);

    NSVGpath*  spath;
    NSVGshape* shape;
    NSVGimage* image = nsvgParseFromFile(paths, "pt", 30);

    mem_release_each(paths, namespath, NULL);

    /* iterate through all shapes three times to render background differently for movement */

    for (uint8_t step = 0; step < 4; step++)
    {

	floatbuffer_reset(scene.vertexbuffer);

	for (shape = image->shapes; shape != NULL; shape = shape->next)
	{

	    uint32_t abgr = shape->fill.color;

	    for (spath = shape->paths; spath != NULL; spath = spath->next)
	    {

		if (spath->closed == 1 && strstr(shape->id, "Pivot") == NULL)
		{

		    vec_t* pointlist = VNEW();
		    vec_t* triangles = VNEW();

		    for (int i = 0; i < spath->npts; i += 3)
		    {

			v2_t* point = CAL(sizeof(v2_t), NULL, NULL);

			point->x = spath->pts[i * 2];
			point->y = -spath->pts[i * 2 + 1];

			point->x += -1.0 + (float) (rand() % 20) / 10.0;
			point->y += -1.0 + (float) (rand() % 20) / 10.0;

			VADD(pointlist, point);
			REL(point);
		    }

		    vec_rem_at_index(pointlist, pointlist->length - 1);
		    triangulate_process(pointlist, triangles);

		    /* add vertexes to floatbuffer */

		    float cf = ogl_color_float_from_argbuint32(abgr);

		    for (int index = 0; index < triangles->length; index += 3)
		    {

			v2_t* p0 = triangles->data[index];
			v2_t* p1 = triangles->data[index + 1];
			v2_t* p2 = triangles->data[index + 2];

			floatbuffer_addvector2(scene.vertexbuffer, *p0);
			floatbuffer_add(scene.vertexbuffer, cf);
			floatbuffer_addvector2(scene.vertexbuffer, *p1);
			floatbuffer_add(scene.vertexbuffer, cf);
			floatbuffer_addvector2(scene.vertexbuffer, *p2);
			floatbuffer_add(scene.vertexbuffer, cf);
		    }

		    REL(triangles);
		    REL(pointlist);
		}
	    }
	}

	gamerenderdata_t data =
	    {
		.buffer = scene.vertexbuffer,
		.layer  = step};

	bus_notify("RND", "UPDBUFF", &data);
    }

    /* create surfaces */

    for (shape = image->shapes; shape != NULL; shape = shape->next)
    {

	for (spath = shape->paths; spath != NULL; spath = spath->next)
	{

	    vec_t* pointlist = VNEW();

	    for (int i = 0; i < spath->npts; i += 3)
	    {

		v2_t* point = CAL(sizeof(v2_t), NULL, NULL);
		point->x    = spath->pts[i * 2];
		point->y    = -spath->pts[i * 2 + 1];

		VADD(pointlist, point);
		REL(point);
	    }

	    if (strstr(shape->id, "Pivot") != NULL)
	    {

		pivot_t* pivot = pivot_alloc(shape->id, v2_init(spath->pts[0], -spath->pts[1]));
		VADD(scene_pivots, pivot);
		REL(pivot);
	    }

	    /* !!! egyszerusiteni ezeket a dolgokat */

	    if (spath->closed == 0 && strstr(shape->id, "Invisibles") == NULL)
	    {
		v2_t* lastpoint = NULL;

		for (int index = 0; index < pointlist->length; index++)
		{
		    v2_t* point = pointlist->data[index];

		    if (lastpoint != NULL)
		    {

			segment2_t* wall = CAL(sizeof(segment2_t), NULL, NULL);
			wall->trans      = *(lastpoint);
			wall->basis      = v2_sub(*point, *lastpoint);

			VADD(scene.debuglines, wall);
			surfaces_addsegment(scene.surfaces, wall);

			REL(wall);
		    }

		    lastpoint = point;
		}
	    }

	    REL(pointlist);
	}
    }

    nsvgDelete(image);

    int roundcount = 0;

    /* setup actors */

    for (int index = 0; index < scene_pivots->length; index++)
    {

	pivot_t* pivot = scene_pivots->data[index];

	uint32_t color     = 0;
	uint32_t isend     = 0;
	uint32_t isgun     = 0;
	uint32_t isinfo    = 0;
	int32_t  xplevel   = 0;
	int32_t  count     = 0;
	int32_t  size      = 0;
	int32_t  direction = 1;

	char* id = pivot->id;
	id += 6;
	char* first = strtok(id, "_");
	while (first != NULL)
	{
	    if (*first == 't')
	    {
		if (first[1] == '0') color = 0xFF0000FF;
		if (first[1] == '1') color = 0x0000FFFF;
		if (first[1] == '2') color = 0x00FF00FF;
		if (first[1] == '3') color = 0xFF00FFFF;
		if (first[1] == '4') color = 0x00FFFFFF;
		if (first[1] == '5') color = 0xFFFF00FF;
	    }
	    if (*first == 'l') xplevel = atoi(&first[1]);
	    if (*first == 's') size = atoi(&first[1]);
	    if (*first == 'c') count = atoi(&first[1]);
	    if (*first == 'd') direction = atoi(&first[1]);
	    if (*first == 'e') isend = 1;
	    if (*first == 'g') isgun = 1;
	    if (*first == 'i') isinfo = 1;

	    first = strtok(NULL, "_");
	}

	if (xplevel == 0)
	{

	    if (isend == 1)
	    {

		scene.endpoint = pivot->position;
	    }
	    else if (isgun == 1)
	    {

		gun_t* gun = gun_alloc(pivot->position, v2_init(10.0, 0.0));
		VADD(scene.guns, gun);
		REL(gun);
	    }
	    else if (isinfo == 1)
	    {

		VADD(scene.tips, pivot);
	    }
	    else
	    {

		actor_modifier_args_t args =
		    {
			.type     = "reset",
			.actor    = scene.herogroup->actor,
			.surfaces = scene.surfaces,
			.gravity  = defaults.gravity,
			.cmdqueue = scene.cmdqueue,
		    };

		if (scene.herogroup->gun)
		    REL(scene.herogroup->gun);
		scene.herogroup->gun = NULL;

		actor_reset(scene.herogroup->actor, pivot->position, &args);

		scene.pan.target_x = scene.pan.x = defaults.width / defaults.scale / 2.0 - pivot->position.x;
		scene.pan.target_y = scene.pan.y = -defaults.height / defaults.scale / 2.0 - pivot->position.y - 80.0;

		//                    actor_t* hero = scene.herogroup->actor;
		//                    hero->metrics.maxhealth += 150.0;
		//                    hero->metrics.maxpower += 150.0;
		//                    hero->power_rate = 100.0;
		//                    hero->health_rate = 100.0;
	    }
	}
	else
	{

	    int enemycount = count;
	    int enemylevel = roundcount + xplevel - 1;

	    if (roundcount > 4) enemycount += 2;
	    else if (roundcount > 2) enemycount += 1;

	    for (int i = 0; i < count; i++)
	    {

		float nx = pivot->position.x + -100.0 + ((float) rand() / (float) RAND_MAX) * 200.0;
		float ny = pivot->position.y;

		v2_t newpos = v2_init(nx, ny);

		base_metrics_t metrics = base_metrics_random();
		metrics.color_a        = color;

		char* name = scene_names->data[rand() % scene_names->length];

		actor_t* hero = scene.herogroup->actor;

		actor_t*      actor = actor_alloc(newpos, name);
		actor_ai_t*   ai    = actor_ai_alloc(actor, color, (float) xplevel / 10.0);
		actor_skin_t* skin  = actor_skin_alloc();

		actor_group_t* group = actor_group_alloc(actor, skin, ai);

		if (color != hero->metrics.color) name = "";

		element_t* hud = actor_hud_element_alloc(name, color, defaults.scale, defaults.font);

		actor_group_sethud(group, hud);

		actor->facing = rand() % 2 == 0 ? kActorFacingLeft : kActorFacingRight;

		actor_updatemetrics(actor, metrics);
		actor_levelup(actor, enemylevel);

		actor_modifier_args_t args =
		    {
			.type          = "reset",
			.actor         = actor,
			.cmdqueue      = scene.cmdqueue,
			.surfaces      = scene.surfaces,
			.gravity       = defaults.gravity,
			.control_state = scene.control_state,
			.ticks         = defaults.ticks};

		actor_reset(actor, newpos, &args);

		VADD(scene.actors, group);

		REL(group);
		REL(actor);
		REL(skin);
		REL(ai);
		REL(hud);
	    }
	}
    }

    REL(scene_pivots);
    REL(scene_names);

    bus_notify("VIEW", "UPDATESKILL", NULL);
}

/* removes actor group from the game */

void scene_remove_actor(actor_t* actor)
{

    if (scene.enemygroup != NULL && actor == scene.enemygroup->actor) scene.enemygroup = NULL;

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* group = scene.actors->data[index];

	if (group->actor == actor)
	{

	    vec_rem_at_index(scene.actors, index);
	    break;
	}
    }
}

/* creates blood and adds it to game */

void scene_create_blood(attack_t* attack)
{
    for (int index = 0; index < 4; index++)
    {

	float nx = -2.0 + (float) (rand() % 40) / 10.0;
	float ny = -2.0 + (float) (rand() % 40) / 10.0;

	v2_t direction = v2_init(nx, ny);
	v2_t basis     = v2_add(attack->basis, direction);

	blood_t* blood = blood_alloc(attack->trans, basis);
	VADD(scene.bloods, blood);
	REL(blood);
    }
}

/* removes blood from the game */

void scene_remove_blood(blood_t* blood)
{
    VREM(scene.bloods, blood);
}

/* update projection */

void scene_update_scene_projection()
{

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    float ratio_a = width / 700.0;
    float ratio_b = height / 600.0;

    if (ratio_a < ratio_b) scene.zoom.ratio = ratio_a;
    else scene.zoom.ratio = ratio_b;

    m4_t pers_matrix = m4_defaultortho(0.0, width / scale, -height / scale, 0.0, 0.0, 1.0);
    m4_t tran_matrix = m4_defaulttranslation(width / 2.0 / scale, -height / 2.0 / scale, 0.0);

    tran_matrix = m4_scale(tran_matrix, scene.zoom.ratio, scene.zoom.ratio, 1.0);
    tran_matrix = m4_translate(tran_matrix, -width / 2.0 / scale, height / 2.0 / scale, 0.0);
    tran_matrix = m4_translate(tran_matrix, scene.pan.x, scene.pan.y, 0.0);

    scene.matrix = m4_multiply(pers_matrix, tran_matrix);

    v4_t vectorlt = v4_init(-100.0, height + 100.0, 0.0, 0.0);
    v4_t vectorrb = v4_init(width + 100.0, -100.0, 0.0, 0.0);

    v3_t nlt = m4_screen_to_world_coords(scene.matrix, vectorlt, width, height);
    v3_t nrb = m4_screen_to_world_coords(scene.matrix, vectorrb, width, height);

    scene.lefttop2world  = v2_init(nlt.x, nlt.y);
    scene.rightbtm2world = v2_init(nrb.x, nrb.y);
}

/* follow hero */

void scene_follow_hero()
{

    actor_t* hero = scene.herogroup->actor;

    float basex = hero->points.base_a.x + (hero->points.base_b.x - hero->points.base_a.x) / 2.0;
    float basey = hero->points.base_a.y + (hero->points.base_b.y - hero->points.base_a.y) / 2.0;

    if (!isnan(basex) && !isnan(basey))
    {

	scene.pan.target_x = defaults.width / defaults.scale / 2.0 - basex;
	scene.pan.target_y = -defaults.height / defaults.scale / 2.0 - basey - 80.0;

	scene.pan.x += (scene.pan.target_x - scene.pan.x) / 8.0;
	scene.pan.y += (scene.pan.target_y - scene.pan.y) / 8.0;

	scene_update_scene_projection();
    }
}

/* default state */

void scene_lines_from_segments(vec_t* segments, floatbuffer_t* vertexbuffer)
{
    float wcoords = ogl_color_float_from_rgbauint32(0xFFFFFFFF);

    /* draw segment lines */

    for (int segmentindex = 0; segmentindex < segments->length; segmentindex++)
    {

	segment2_t* segment = segments->data[segmentindex];

	floatbuffer_addvector2(vertexbuffer, segment->trans);
	floatbuffer_add(vertexbuffer, wcoords);
	floatbuffer_addvector2(vertexbuffer, v2_add(segment->trans, segment->basis));
	floatbuffer_add(vertexbuffer, wcoords);
    }
}

/* level stepping */

void scene_update(float delta)
{

    uint8_t clear = 1;

    /* generate new actor states */

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* group = scene.actors->data[index];

	actor_modifier_args_t args =
	    {
		.type          = NULL,
		.actor         = group->actor,
		.cmdqueue      = scene.cmdqueue,
		.surfaces      = scene.surfaces,
		.gravity       = defaults.gravity,
		.control_state = scene.control_state,
		.ticks         = defaults.ticks,
		.delta         = delta};

	actor_t* actor = group->actor;

	if (index > 0)
	{

	    // move other actors with ai

	    control_state_t nullstate = {0};

	    args.control_state = nullstate;

	    actor_ai_new(group->ai, &args, scene.actors);

	    actor_t* hero = scene.herogroup->actor;

	    if (actor->metrics.color != hero->metrics.color && actor->state != kActorStateDeath)
	    {

		clear = 0;
	    }
	}
	else
	{

	    actor_new(group->actor, &args);

	    char scnlive = scene.state == kSceneStateAlive;
	    char actdead = actor->state == kActorStateDeath;

	    if (scnlive && actdead)
	    {

		scene.state = kSceneStateDead;
		actor_savestate(scene.herogroup->actor);
	    }

	    char endxok = fabs(actor->points.base_a.x - scene.endpoint.x) < 100.0;
	    char endyok = fabs(actor->points.base_a.y - scene.endpoint.y) < 100.0;

	    if (scnlive && endxok && endyok)
	    {

		if (scene.game_clear == 1)
		{

		    scene.state = kSceneStateFinished;
		    cmdqueue_add(scene.cmdqueue, "scene.showfinished", NULL, NULL);
		}
		else if (scene.game_clear_showed == 0)
		{

		    scene.game_clear_showed = 1;
		    cmdqueue_add(scene.cmdqueue, "scene.shownotclear", NULL, NULL);
		}
	    }
	}

	/* update fainted if exists */

	if (group->fainted != NULL)
	{

	    actor_group_t*            fainted = group->fainted;
	    actor_t*                  body    = fainted->actor;
	    actor_modifier_ragdoll_t* data    = body->ik->data;

	    body->points.neck = actor->points.head;
	    body->points.neck.x -= data->dis.necktohip->distance / 2.0;
	    body->points.neck.y += 20.0;

	    body->points.hip = actor->points.head;
	    body->points.hip.x += data->dis.necktohip->distance / 2.0;
	    body->points.hip.y += 20.0;

	    data->mas.neck->basis = v2_init(0.0, 0.0);
	    data->mas.hip->basis  = v2_init(0.0, 0.0);
	}

	/* update gun if exists */

	if (group->gun != NULL)
	{

	    gun_t* gun       = group->gun;
	    gun->facing      = actor->facing;
	    gun->mass->trans = actor->points.hand_a;
	    gun->mass->basis = v2_sub(actor->points.hand_a, actor->points.elbow_a);
	}
    }

    /* set clear */

    scene.game_clear = clear;

    /* update bloods */

    for (int index = 0; index < scene.bloods->length; index++)
    {

	blood_t* blood = scene.bloods->data[index];

	blood_new(blood, scene.surfaces, &defaults.gravity, scene.cmdqueue, delta);
    }

    /* follow hero */

    scene_follow_hero();

    /* increase skin phase */

    scene.phase += delta / 10.0;
    if (scene.phase > 3.0) scene.phase = 0.0;

    /* upload vertexes to renderer */

    floatbuffer_reset(scene.vertexbuffer);

    /* draw guns */

    if (scene.guns->length > 0)
    {

	for (int index = 0; index < scene.guns->length; index++)
	{

	    gun_t* gun = scene.guns->data[index];
	    gun_new_skin(gun, scene.vertexbuffer);
	}
    }

    /* draw actors */

    for (int index = 1; index < scene.actors->length; index++)
    {

	actor_group_t* group = scene.actors->data[index];

	actor_t* actor = group->actor;

	if (actor->points.hip.x > scene.lefttop2world.x - 50.0 &&
	    actor->points.hip.x < scene.rightbtm2world.x + 50.0 &&
	    actor->points.hip.y < scene.lefttop2world.y - 50.0 &&
	    actor->points.hip.y > scene.rightbtm2world.y + 50.0)
	{

	    actor_skin_new(group->skin, group->actor, scene.vertexbuffer, (int) scene.phase);
	}
    }

    /* draw hero for last */

    actor_skin_new(scene.herogroup->skin, scene.herogroup->actor, scene.vertexbuffer, (int) scene.phase);

    gamerenderdata_t data = (gamerenderdata_t){
	.buffer = scene.vertexbuffer,
	.layer  = 3};

    bus_notify("RND", "UPDBUFF", &data);

    /* draw blood blood */

    floatbuffer_reset(scene.vertexbuffer);

    if (scene.bloods->length > 0)
    {

	for (int index = 0; index < scene.bloods->length; index++)
	{

	    blood_t* blood = scene.bloods->data[index];

	    if (blood->mass->trans.x > scene.lefttop2world.x &&
		blood->mass->trans.x < scene.rightbtm2world.x &&
		blood->mass->trans.y < scene.lefttop2world.y &&
		blood->mass->trans.y > scene.rightbtm2world.y)
	    {

		blood_new_skin(blood, scene.vertexbuffer);
	    }
	}

	gamerenderdata_t data =
	    {
		.buffer = scene.vertexbuffer,
		.layer  = 5};

	bus_notify("RND", "UPDBUFF", &data);
    }

    if (defaults.debug_mode == 1)
    {

	// debug points

	for (int index = 0; index < scene.actors->length; index++)
	{

	    actor_group_t* group = scene.actors->data[index];
	    actor_t*       actor = group->actor;

	    if (actor->points.hip.x > scene.lefttop2world.x - 50.0 &&
		actor->points.hip.x < scene.rightbtm2world.x + 50.0 &&
		actor->points.hip.y < scene.lefttop2world.y - 50.0 &&
		actor->points.hip.y > scene.rightbtm2world.y + 50.0)
	    {

		actor_debug_points(actor, scene.vertexbuffer);
	    }
	}

	data = (gamerenderdata_t){
	    .buffer = scene.vertexbuffer,
	    .layer  = 5};

	bus_notify("RND", "UPDBUFF", &data);

	/* draw bones and forces */

	floatbuffer_reset(scene.vertexbuffer);

	scene_lines_from_segments(scene.debuglines, scene.vertexbuffer);

	for (int index = 0; index < scene.actors->length; index++)
	{
	    actor_group_t* group = scene.actors->data[index];
	    actor_t*       actor = group->actor;

	    if (actor->points.hip.x > scene.lefttop2world.x - 50.0 &&
		actor->points.hip.x < scene.rightbtm2world.x + 50.0 &&
		actor->points.hip.y < scene.lefttop2world.y - 50.0 &&
		actor->points.hip.y > scene.rightbtm2world.y + 50.0)
	    {

		actor_debug_lines(group->actor, scene.vertexbuffer);
		actor_ai_debug_lines(group->ai, group->ai, scene.vertexbuffer);
	    }
	}

	data = (gamerenderdata_t){
	    .buffer = scene.vertexbuffer,
	    .layer  = 4};

	bus_notify("RND", "UPDBUFF", &data);
    }
    else
    {
	floatbuffer_reset(scene.vertexbuffer);

	//			gamerenderdata_t data =
	//			{
	//				.buffer = scene.vertexbuffer ,
	//				.layer = 5
	//			};
	//
	//			bus_notify( "RND" , "UPDBUFF" , &data );

	data.layer = 4;

	bus_notify("RND", "UPDBUFF", &data);
    }

    /* check info pivots */

    for (int index = 0; index < scene.tips->length; index++)
    {

	actor_t* actor = scene.herogroup->actor;
	pivot_t* pivot = scene.tips->data[index];

	v2_t delta = v2_sub(actor->points.hip, pivot->position);

	if (fabs(delta.x) < 100.0 && fabs(delta.y) < 100.0)
	{

	    bus_notify("VIEW", "SHOWTIP", pivot->id);
	    vec_rem_at_index(scene.tips, index);
	    break;
	}
    }

    /* check collected actions */

    if (scene.cmdqueue->commands->length > 0)
    {

	for (int index = 0; index < scene.cmdqueue->commands->length; index++)
	{

	    cmd_t* command = (cmd_t*) scene.cmdqueue->commands->data[index];

	    // printf( "%s\n" , command->name );

	    if (strcmp(command->name, "scene.kick") == 0) scene_kick(command->args);
	    else if (strcmp(command->name, "scene.punch") == 0) scene_punch(command->args);
	    else if (strcmp(command->name, "scene.shoot") == 0) scene_shoot(command->args);
	    else if (strcmp(command->name, "scene.pickup") == 0) scene_pickup(command->data);
	    else if (strcmp(command->name, "scene.removeactor") == 0) scene_remove_actor(command->data);
	    else if (strcmp(command->name, "scene.removeblood") == 0) scene_remove_blood(command->data);
	    else if (strcmp(command->name, "scene.showwasted") == 0) bus_notify("VIEW", "SHOWWASTED", command->data);
	    else if (strcmp(command->name, "scene.showfinished") == 0) bus_notify("VIEW", "SHOWELEMENT", (char*) "finishedelement");
	    else if (strcmp(command->name, "updateskill") == 0) bus_notify("VIEW", "UPDATESKILL", NULL);
	}
	cmdqueue_reset(scene.cmdqueue);
    }
}

/* attack hits actor */

char scene_hit(attack_t* attack)
{

    /* get closest enemy */

    float          maxdist = -1.0;
    actor_t*       sender  = attack->actor;
    actor_group_t* group   = NULL;

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* enemygroup = (actor_group_t*) scene.actors->data[index];
	actor_t*       enemy      = enemygroup->actor;

	char colorok = 0;

	if (sender->state == kActorStateDeath) colorok = 1;
	else if (enemy->metrics.color != sender->metrics.color) colorok = 1;

	if (enemy != attack->actor && enemy->state != kActorStateDeath && colorok == 1)
	{

	    float dx = fabs(enemy->points.head.x - attack->trans.x);
	    float dy = fabs(enemy->points.head.y - attack->trans.y);

	    if (maxdist < 0.0 || dx + dy < maxdist)
	    {

		maxdist = dx + dy;
		group   = enemygroup;
	    }
	}
    }

    if (group != NULL)
    {

	actor_t* enemy = group->actor;

	v2_t isp;
	char bone = actor_hit(enemy, attack, &isp);

	if (bone > 0)
	{
	    // printf( "%s health %f level %i hits %s health %f level %i power %f\n" , sender->name , sender->health , sender->metrics.level , enemy->name , enemy->health , enemy->metrics.level , attack->power);

	    actor_modifier_args_t args =
		{
		    .type          = NULL,
		    .actor         = enemy,
		    .cmdqueue      = scene.cmdqueue,
		    .surfaces      = scene.surfaces,
		    .gravity       = defaults.gravity,
		    .control_state = scene.control_state,
		    .ticks         = defaults.ticks};

	    char success = actor_modifier_ragdoll_hit(enemy->ik, &args, attack, bone, isp);

	    if (bone == kActorBoneHead && success == 1)
	    {

		if (sender->state == kActorStateJump)
		{

		    cmdqueue_add(scene.cmdqueue, "ui.showheadkick", NULL, NULL);
		}

		attack_t* bloodattack = attack_alloc(attack->actor, isp, v2_resize(attack->basis, 3.0), attack->power);
		scene_create_blood(bloodattack);
		REL(bloodattack);
	    }

	    for (uint32_t groupindex = 0; groupindex < scene.actors->length; groupindex++)
	    {

		actor_group_t* onegroup = scene.actors->data[groupindex];

		if (onegroup->actor == attack->actor)
		{

		    actor_ai_set_enemy(group->ai, onegroup);
		    break;
		}
	    }

	    if (attack->power < 1000.0) bus_notify("SND", "PLAYSND", "hit");

	    if (enemy->state == kActorStateDeath)
	    {

		bus_notify("SND", "PLAYSND", "death");

		float xpgain = (float) enemy->metrics.level * 50;

		sender->metrics.xp += xpgain;

		bus_notify("VIEW", "REMOVEHUD", group);

		if (sender->metrics.xp >= 1000.0)
		{

		    sender->metrics.xp = 0.0;
		    actor_levelup(sender, 1);

		    if (sender == scene.herogroup->actor)
		    {

			cmdqueue_add(scene.cmdqueue, "updateskill", NULL, NULL);
		    }
		}
	    }
	    else
	    {

		bus_notify("SND", "PLAYSND", "argh");
	    }

	    /* save enemy for enemybar/label */

	    if (attack->actor == scene.herogroup->actor) scene.enemygroup = group;

	    return 1;
	}
    }

    return 0;
}

/* kick arrived from an actor, check hit */

void scene_kick(attack_t* attack)
{

    /* actor_t* actor   = attack->actor; */
    char success = scene_hit(attack);

    if (success == 1)
    {

	//            if 		( actor->state == kActorStateKick ) actor_modifier_kick_stopkick( actor->kick , attack->actor );
	//            else if ( actor->state == kActorStateJump ) actor_modifier_jump_stopkick( actor->jump );
    }
}

/* punch arrived from an actor, check hit */

void scene_punch(attack_t* attack)
{
    /* get actor's group */

    actor_t* actor = attack->actor;

    actor_group_t* group = NULL;

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* onegroup = scene.actors->data[index];

	if (onegroup->actor == attack->actor)
	{

	    group = onegroup;
	    break;
	}
    }

    if (group != NULL)
    {

	if (group->fainted != NULL)
	{

	    actor_group_t*            fainted = group->fainted;
	    actor_t*                  body    = fainted->actor;
	    actor_modifier_ragdoll_t* data    = body->ik->data;

	    data->mas.neck->basis = v2_init(actor->speed.x * 1.0, 10.0);
	    data->mas.hip->basis  = v2_init(actor->speed.x * 1.0, 10.0);
	    actor_modifier_ragdoll_setdragged(body->ik, 0);

	    fainted->isgrabbed = 0;

	    actor_group_setfainted(group, NULL);
	}
	else
	{

	    char success = scene_hit(attack);

	    if (success == 1)
	    {

		if (actor->state == kActorStateWalk) actor_modifier_walk_stoppunch(actor->walk);
		else if (actor->state == kActorStateJump) actor_modifier_jump_stoppunch(actor->jump);
	    }
	}
    }
}

/* shoot arrived from an actor, check hit */

void scene_shoot(attack_t* attack)
{

    actor_t* actor = attack->actor;

    actor_group_t* group = NULL;

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* onegroup = scene.actors->data[index];

	if (onegroup->actor == attack->actor)
	{

	    group = onegroup;
	    break;
	}
    }

    if (group != NULL)
    {

	if (group->gun != NULL)
	{

	    gun_t* gun = group->gun;

	    if (gun->bullets > 0)
	    {

		gun->bullets -= 1;
		gun->shot = 1;

		bus_notify("VIEW", "UPDATEBULLETS", group);

		attack->basis = v2_init(actor->facing * 1000.0, 0.0);
		// attack->power = 50.0;

		gun->mass->basis = attack->basis;

		bus_notify("SND", "PLAYSND", "shoot");

		char success = scene_hit(attack);

		if (success == 1)
		{

		    if (actor->state == kActorStateWalk) actor_modifier_walk_stoppunch(actor->walk);
		    else if (actor->state == kActorStateJump) actor_modifier_jump_stoppunch(actor->jump);
		}
	    }
	}
    }
}

/* punch arrived from an actor, check hit */

void scene_pickup(actor_t* actor)
{

    if (defaults.sceneindex > 3 && defaults.donation_arrived == 0)
    {
	bus_notify("VIEW", "SHOWTIP", "PlsGive");
	return;
    }

    /* get actor's group */

    actor_group_t* group = NULL;

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* onegroup = scene.actors->data[index];

	if (onegroup->actor == actor)
	{

	    group = onegroup;
	    break;
	}
    }

    if (group->fainted != NULL) return;

    /* look for dead body */

    for (int index = 0; index < scene.actors->length; index++)
    {

	actor_group_t* onegroup = (actor_group_t*) scene.actors->data[index];

	actor_t* enemy = onegroup->actor;

	v2_t delta = v2_sub(actor->points.hip, enemy->points.hip);

	if (fabs(delta.x) < 100.0 && fabs(delta.y) < 100.0 && enemy->state == kActorStateDeath && onegroup->isgrabbed == 0)
	{

	    actor_modifier_ragdoll_setdragged(enemy->ik, 1);
	    actor_group_setfainted(group, onegroup);
	    onegroup->isgrabbed = 1;

	    actor_group_t*            fainted = group->fainted;
	    actor_t*                  body    = fainted->actor;
	    actor_modifier_ragdoll_t* data    = body->ik->data;

	    body->points.neck = actor->points.head;
	    body->points.neck.x -= data->dis.necktohip->distance / 2.0;
	    body->points.neck.y += 20.0;

	    body->points.hip = actor->points.head;
	    body->points.hip.x += data->dis.necktohip->distance / 2.0;
	    body->points.hip.y += 20.0;

	    body->points.knee_a = body->points.hip;
	    body->points.knee_b = body->points.hip;

	    body->points.ankle_a = body->points.hip;
	    body->points.ankle_b = body->points.hip;

	    body->points.elbow_a = body->points.neck;
	    body->points.elbow_b = body->points.neck;

	    body->points.hand_a = body->points.neck;
	    body->points.hand_b = body->points.neck;

	    break;
	}
    }

    /* look for gun */

    for (int index = 0; index < scene.guns->length; index++)
    {

	gun_t* gun = scene.guns->data[index];

	v2_t delta = v2_sub(actor->points.hip, gun->mass->trans);

	if (fabs(delta.x) < 100.0 && fabs(delta.y) < 100.0)
	{

	    actor_group_setgun(group, gun);
	    bus_notify("VIEW", "UPDATEBULLETS", group);
	}
    }
}

/* key press event */

void scene_key_statechange(SDL_Keycode code, char state)
{

    if (code == SDLK_LEFT) scene.control_state.left_pressed = state;
    else if (code == SDLK_RIGHT) scene.control_state.right_pressed = state;
    else if (code == SDLK_UP) scene.control_state.jump_pressed = state;
    else if (code == SDLK_DOWN) scene.control_state.squat_pressed = state;
    else if (code == SDLK_SPACE) scene.control_state.run_pressed = state;
    else if (code == SDLK_d) scene.control_state.kick_pressed = state;
    else if (code == SDLK_s) scene.control_state.block_pressed = state;
    else if (code == SDLK_f) scene.control_state.punch_pressed = state;
    else if (code == SDLK_c) scene.control_state.shoot_pressed = state;
}

/* message arrived on scene channel */

void scene_onmessage(const char* name, void* data)
{

    if (strcmp(name, "UPDATE") == 0)
    {

	cmdqueue_timer(scene.cmdqueue, defaults.ticks);

	float ratio = *(float*) data;
	scene_update(ratio);
    }
    else if (strcmp(name, "RENDER") == 0)
    {

	scene_update_scene_projection();

	gamerenderdata_t data = (gamerenderdata_t){
	    .matrix = scene.matrix,
	    .layers = {(int) scene.phase, 3, 4, 5, 0}};

	bus_notify("RND", "RNDBUFF", &data);
    }
    else if (strcmp(name, "KEYDOWN") == 0)
    {

	SDL_Keycode code = *(SDL_Keycode*) data;
	scene_key_statechange(code, 1);
    }
    else if (strcmp(name, "KEYUP") == 0)
    {

	SDL_Keycode code = *(SDL_Keycode*) data;
	scene_key_statechange(code, 0);
    }
    else if (strcmp(name, "LOAD") == 0)
    {

	int level = *(int*) data;
	scene_load(level);
    }
}

#endif
