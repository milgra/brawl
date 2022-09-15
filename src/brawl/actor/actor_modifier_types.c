#ifndef actor_modifier_types_h
#define actor_modifier_types_h

#include "cmd.c"
#include "defaults.c"
#include "physics2.c"
#include "zc_util2.c"

typedef struct _control_state_t control_state_t;
struct _control_state_t
{
    char left_pressed;
    char right_pressed;

    char jump_pressed;
    char squat_pressed;

    char run_pressed;
    char kick_pressed;
    char punch_pressed;
    char block_pressed;
    char shoot_pressed;
};

typedef struct _actor_metrics_t actor_metrics_t;
struct _actor_metrics_t
{
    float headlength;
    float bodylength;
    float armlength;
    float leglength;

    float headwidth;
    float neckwidth;
    float armwidth;
    float bodywidth;
    float hipwidth;
    float legwidth;

    float walkspeed;
    float runspeed;
    float punchspeed;
    float kickspeed;

    float maxpower;
    float maxhealth;

    float hitpower;
    float kickpower;

    uint8_t  level;
    uint32_t xp;
    uint32_t color;

    uint32_t color_a;
    uint32_t color_b;
    uint32_t color_c;
    uint32_t color_d;
};

typedef struct _actor_points_t actor_points_t;
struct _actor_points_t
{
    v2_t head;
    v2_t neck;
    v2_t hip;

    v2_t hand_a;
    v2_t hand_b;

    v2_t elbow_a;
    v2_t elbow_b;

    v2_t knee_a;
    v2_t knee_b;

    v2_t ankle_a;
    v2_t ankle_b;

    v2_t base_a;
    v2_t base_b;
};

typedef struct _actor_modifier_args_t actor_modifier_args_t;
struct _actor_modifier_args_t
{
    char* type;
    void* actor;

    cmdqueue_t* cmdqueue;
    surfaces_t* surfaces;

    v2_t gravity;

    control_state_t control_state;
    uint32_t        ticks;

    float delta;
};

typedef struct _actor_modifier_t actor_modifier_t;
struct _actor_modifier_t
{
    char* type;
    void* data;
    void (*_new)(actor_modifier_t* modifier, actor_modifier_args_t* arguments);
};

typedef struct _attack_t attack_t;
struct _attack_t
{
    void* actor;
    float power;
    v2_t  trans;
    v2_t  basis;
};

attack_t* attack_alloc(void* actor, v2_t trans, v2_t basis, float power);
void      attack_destruct(void* pointer);

typedef struct _movement_t movement_t;
struct _movement_t
{
    char autostop;

    float size;
    float speed;

    float minimum;
    float maximum;

    float ratio;
};

movement_t movement_alloc(float size, float speed, float minimum, float maximum, char autostop);
void       movement_step(movement_t* movement, float delta);

typedef struct _sight_t sight_t;
struct _sight_t
{
    v2_t final_point;
    v2_t basis_lower;
    v2_t basis_upper;
    v2_t trans;
};

#endif /* actor_type_h */

#if __INCLUDE_LEVEL__ == 0

attack_t* attack_alloc(void* actor, v2_t trans, v2_t basis, float power)
{
    attack_t* attack = CAL(sizeof(attack_t), attack_destruct, NULL);
    attack->actor    = actor;
    attack->trans    = trans;
    attack->basis    = basis;
    attack->power    = power;

    if (actor != NULL) RET(actor);
    return attack;
}

/* cleanup */

void attack_destruct(void* pointer)
{
    attack_t* attack = pointer;
    if (attack->actor != NULL) REL(attack->actor);
}

/* movement */

movement_t movement_alloc(float size, float speed, float minimum, float maximum, char autostop)
{
    movement_t result = {
	.size     = size,
	.speed    = speed,
	.minimum  = minimum,
	.maximum  = maximum,
	.autostop = autostop};
    return result;
}

/* movement step */

void movement_step(movement_t* movement, float delta)
{
    if (movement->speed != 0.0)
    {
	movement->size += movement->speed * delta;

	if (movement->size > movement->maximum)
	{
	    movement->size = movement->maximum;
	    if (movement->autostop == 1) movement->speed *= -1.0;
	}
	if (movement->size < movement->minimum) movement->speed = 0.0;
    }
}

#endif
