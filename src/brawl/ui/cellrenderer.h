#ifndef cellrenderer_h
#define cellrenderer_h

#include "element.h"
#include "zc_channel.c"
#include <pthread.h>
#include <sys/time.h>

typedef struct _cellrenderer_t cellrenderer_t;
struct _cellrenderer_t
{
    char      alive;
    pthread_t thread;

    input_t input;
    ch_t*   elements_to_render;
};

cellrenderer_t* cellrenderer_alloc(font_t* font, char* respath);

void cellrenderer_queue(cellrenderer_t* renderer, element_t* element);

#endif
