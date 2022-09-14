
#ifndef actor_group_h
#define actor_group_h

#include "zc_string.c"
#include <stdio.h>

typedef struct _actor_group_t actor_group_t;
struct _actor_group_t
{
    void* actor;
    void* skin;
    void* ai;

    void* hud;
    void* gun;
    void* bubble;
    void* fainted;

    char isgrabbed;

    uint32_t removetime;

    str_t* currenttext;
};

actor_group_t* actor_group_alloc(void* actor, void* skin, void* ai);
void           actor_group_destruct(void* pointer);
void           actor_group_setfainted(actor_group_t* group, void* fainted);
void           actor_group_setbubble(actor_group_t* group, void* bubble);
void           actor_group_sethud(actor_group_t* group, void* hud);
void           actor_group_setgun(actor_group_t* group, void* gun);

#endif /* actor_group_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_group.c"
#include "zc_memory.c"

/* actor group default state */

actor_group_t* actor_group_alloc(void* actor, void* skin, void* ai)
{
    actor_group_t* result = CAL(sizeof(actor_group_t), actor_group_destruct, NULL);
    result->actor         = RET(actor);
    result->skin          = RET(skin);
    result->ai            = RET(ai);

    result->hud     = NULL;
    result->gun     = NULL;
    result->bubble  = NULL;
    result->fainted = NULL;

    result->isgrabbed  = 0;
    result->removetime = 0;

    return result;
}

/* cleanup */

void actor_group_destruct(void* pointer)
{
    actor_group_t* group = pointer;

    if (group->skin)
	REL(group->skin);
    if (group->actor)
	REL(group->actor);
    if (group->ai)
	REL(group->ai);

    if (group->hud)
	REL(group->hud);
    if (group->gun)
	REL(group->gun);
    if (group->bubble)
	REL(group->bubble);
    if (group->fainted)
	REL(group->fainted);
}

void actor_group_setfainted(actor_group_t* group, void* fainted)
{
    if (group->fainted) REL(group->fainted);
    group->fainted = NULL;
    if (fainted) group->fainted = RET(fainted);
}

void actor_group_setbubble(actor_group_t* group, void* bubble)
{
    if (group->bubble) REL(group->bubble);
    group->bubble = NULL;
    if (bubble) group->bubble = RET(bubble);
}

void actor_group_setgun(actor_group_t* group, void* gun)
{
    if (group->gun) REL(group->gun);
    group->gun = NULL;
    if (gun) group->gun = RET(gun);
}

void actor_group_sethud(actor_group_t* group, void* hud)
{
    if (group->hud) REL(group->hud);
    group->hud = NULL;
    if (hud) group->hud = RET(hud);
}

#endif
