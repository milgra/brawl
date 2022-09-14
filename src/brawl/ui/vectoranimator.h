

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
