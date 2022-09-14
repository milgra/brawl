
#ifndef actor_group_h
#define actor_group_h

#include "mtstr.c"
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

    mtstr_t* currenttext;
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
#include "mtmem.c"

/* actor group default state */

actor_group_t* actor_group_alloc(void* actor, void* skin, void* ai)
{
    actor_group_t* result = mtmem_calloc(sizeof(actor_group_t), actor_group_destruct);
    result->actor         = mtmem_retain(actor);
    result->skin          = mtmem_retain(skin);
    result->ai            = mtmem_retain(ai);

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

    mtmem_release(group->skin);
    mtmem_release(group->actor);
    mtmem_release(group->ai);

    mtmem_release(group->hud);
    mtmem_release(group->gun);
    mtmem_release(group->bubble);
    mtmem_release(group->fainted);
}

void actor_group_setfainted(actor_group_t* group, void* fainted)
{
    mtmem_release(group->fainted);
    group->fainted = mtmem_retain(fainted);
}

void actor_group_setbubble(actor_group_t* group, void* bubble)
{
    mtmem_release(group->bubble);
    group->bubble = mtmem_retain(bubble);
}

void actor_group_setgun(actor_group_t* group, void* gun)
{
    mtmem_release(group->gun);
    group->gun = mtmem_retain(gun);
}

void actor_group_sethud(actor_group_t* group, void* hud)
{
    mtmem_release(group->hud);
    group->hud = mtmem_retain(hud);
}

#endif
