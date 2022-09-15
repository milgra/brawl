#ifndef base_metrics_h
#define base_metrics_h

#include "zc_memory.c"
#include <stdio.h>
#include <stdlib.h>

typedef struct _base_metrics_t base_metrics_t;
struct _base_metrics_t
{
    float height;
    float hitpower;
    float hitrate;
    float stamina;
    float speed;

    uint32_t color_a;
    uint32_t color_b;
};

base_metrics_t base_metrics_default(void);
base_metrics_t base_metrics_random(void);

void base_metrics_update(base_metrics_t* metrics, float height, float hitpower, float hitrate, float stamina, float speed);

#endif /* base_metrics_h */

#if __INCLUDE_LEVEL__ == 0

#include "base_metrics.c"

static uint32_t* colors = NULL;

/* reset metrics to defaults */

base_metrics_t base_metrics_default()
{
    base_metrics_t metrics;

    metrics.height   = 0.5;
    metrics.hitpower = 0.5;
    metrics.hitrate  = 0.5;
    metrics.stamina  = 0.5;
    metrics.speed    = 0.5;
    metrics.color_a  = 0xFF0000FF;
    metrics.color_b  = 0x000000FF;

    return metrics;
}

/* randomize metrics */

base_metrics_t base_metrics_random()
{
    if (colors == NULL)
    {
	colors = CAL(sizeof(uint32_t) * 30, NULL, NULL);
	for (int index = 0; index < 30; index++) colors[index] = (rand() % 0xFFFFFFFF) | 0xFF;
    }
    base_metrics_t metrics;

    metrics.height   = (float) (rand() % 10) / 10.0;
    metrics.hitpower = (float) (rand() % 10) / 10.0;
    metrics.hitrate  = 0.0;
    metrics.stamina  = (float) (rand() % 10) / 10.0;
    metrics.speed    = 0.0;
    metrics.color_a  = colors[rand() % 30];
    metrics.color_b  = colors[rand() % 30];

    metrics.hitrate = (2.5 - metrics.height) / 2.0 - metrics.hitpower;
    metrics.speed   = (2.5 - metrics.height) / 2.0 - metrics.stamina;

    return metrics;
}

/* update specific value with delta */

void base_metrics_update(base_metrics_t* metrics, float height, float hitpower, float hitrate, float stamina, float speed)
{
    metrics->height += height;
    metrics->hitpower += -height / 4.0 + hitpower - hitrate;
    metrics->hitrate += -height / 4.0 + hitrate - hitpower;
    metrics->stamina += -height / 4.0 + stamina - speed;
    metrics->speed += -height / 4.0 + speed - stamina;

    float delta = (2.5 - metrics->height) / 2.0;

    if (metrics->hitpower > 1.0) metrics->hitrate = delta - metrics->hitpower;
    if (metrics->hitrate > 1.0) metrics->hitpower = delta - metrics->hitrate;
    if (metrics->stamina > 1.0) metrics->speed = delta - metrics->stamina;
    if (metrics->speed > 1.0) metrics->stamina = delta - metrics->speed;

    if (metrics->hitpower < 0.0) metrics->hitrate = delta;
    if (metrics->hitrate < 0.0) metrics->hitpower = delta;
    if (metrics->stamina < 0.0) metrics->speed = delta;
    if (metrics->speed < 0.0) metrics->stamina = delta;
}

#endif
