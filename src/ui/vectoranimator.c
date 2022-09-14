

#ifndef vectoranimator_h
#define vectoranimator_h

#include "math1.c"
#include "zc_util2.c"
#include <stdio.h>

#define kAnimationTypeLinear 0
#define kAnimationTypeEaseInQuadratic 1
#define kAnimationTypeEaseOutQuadratic 2
#define kAnimationTypeEaseInEaseOutQuadratic 3
#define kAnimationTypeEaseInCubic 4
#define kAnimationTypeEaseOutCubic 5
#define kAnimationTypeEaseInEaseOutCubic 6
#define kAnimationTypeEaseInQuartic 7
#define kAnimationTypeEaseOutQuartic 8
#define kAnimationTypeEaseInEaseOutQuartic 9
#define kAnimationTypeEaseInQuintic 10
#define kAnimationTypeEaseOutQuintic 11
#define kAnimationTypeEaseInEaseOutQuintic 12
#define kAnimationTypeEaseInSine 13
#define kAnimationTypeEaseOutSine 14
#define kAnimationTypeEaseInEaseOutSine 15
#define kAnimationTypeEaseInExp 16
#define kAnimationTypeEaseOutExp 17
#define kAnimationTypeEaseInEaseOutExp 18
#define kAnimationTypeEaseInCirc 19
#define kAnimationTypeEaseOutCirc 20
#define kAnimationTypeEaseInEaseOutCirc 21

typedef struct _vectoranimator_t vectoranimator_t;
struct _vectoranimator_t
{
    char type;
    char running;

    uint32_t starttime;
    uint32_t endtime;
    uint32_t duration;

    v2_t startvalue;
    v2_t endvalue;
    v2_t actual;
    v2_t delta;
};

void vectoranimator_init(vectoranimator_t* animator, v2_t startvalue, v2_t endvalue, uint32_t starttime, uint32_t endtime, char type);

v2_t vectoranimator_step(vectoranimator_t* animator, uint32_t time);

void vectoranimator_stop(vectoranimator_t* animator);

#endif

#if __INCLUDE_LEVEL__ == 0

/* init animator */

void vectoranimator_init(vectoranimator_t* animator, v2_t startvalue, v2_t endvalue, uint32_t starttime, uint32_t endtime, char type)
{
    animator->type    = type;
    animator->running = 0;

    animator->startvalue = startvalue;
    animator->endvalue   = endvalue;
    animator->actual     = startvalue;
    animator->delta      = v2_sub(endvalue, startvalue);

    animator->starttime = starttime;
    animator->endtime   = endtime;
    animator->duration  = endtime - starttime;
}

/* stops animator */

void vectoranimator_stop(vectoranimator_t* animator)
{
    animator->running   = 0;
    animator->starttime = 0;
    animator->endtime   = 0;
}

/* iterates animator */

v2_t vectoranimator_step(vectoranimator_t* animator, uint32_t time)
{
    if (time >= animator->starttime && time <= animator->endtime)
    {
	animator->running = 1;

	float (*tween)(float start, float delta, float time, float duration) = NULL;

	switch (animator->type)
	{
	    case kAnimationTypeLinear: tween = float_tween_linear; break;

	    case kAnimationTypeEaseInQuadratic: tween = float_tween_ease_in_quadratic; break;
	    case kAnimationTypeEaseOutQuadratic: tween = float_tween_ease_out_quadratic; break;
	    case kAnimationTypeEaseInEaseOutQuadratic: tween = float_tween_ease_in_ease_out_quadratic; break;

	    case kAnimationTypeEaseInCubic: tween = float_tween_ease_in_cubic; break;
	    case kAnimationTypeEaseOutCubic: tween = float_tween_ease_out_cubic; break;
	    case kAnimationTypeEaseInEaseOutCubic: tween = float_tween_ease_in_ease_out_cubic; break;

	    case kAnimationTypeEaseInQuartic: tween = float_tween_ease_in_quartic; break;
	    case kAnimationTypeEaseOutQuartic: tween = float_tween_ease_out_quartic; break;
	    case kAnimationTypeEaseInEaseOutQuartic: tween = float_tween_ease_in_ease_out_quartic; break;

	    case kAnimationTypeEaseInQuintic: tween = float_tween_ease_in_quintic; break;
	    case kAnimationTypeEaseOutQuintic: tween = float_tween_ease_out_quintic; break;
	    case kAnimationTypeEaseInEaseOutQuintic: tween = float_tween_ease_in_ease_out_quintic; break;

	    case kAnimationTypeEaseInSine: tween = float_tween_ease_in_sine; break;
	    case kAnimationTypeEaseOutSine: tween = float_tween_ease_out_sine; break;
	    case kAnimationTypeEaseInEaseOutSine: tween = float_tween_ease_in_ease_out_sine; break;

	    case kAnimationTypeEaseInExp: tween = float_tween_ease_in_exp; break;
	    case kAnimationTypeEaseOutExp: tween = float_tween_ease_out_exp; break;
	    case kAnimationTypeEaseInEaseOutExp: tween = float_tween_ease_in_ease_out_exp; break;

	    case kAnimationTypeEaseInCirc: tween = float_tween_ease_in_circ; break;
	    case kAnimationTypeEaseOutCirc: tween = float_tween_ease_out_circ; break;
	    case kAnimationTypeEaseInEaseOutCirc: tween = float_tween_ease_in_ease_out_circ; break;
	}

	animator->actual.x = (*tween)(animator->startvalue.x, animator->delta.x, time - animator->starttime, animator->duration);
	animator->actual.y = (*tween)(animator->startvalue.y, animator->delta.y, time - animator->starttime, animator->duration);
    }
    else
    {
	animator->actual  = v2_add(animator->startvalue, animator->delta);
	animator->running = 0;
    }

    return animator->actual;
}

#endif
