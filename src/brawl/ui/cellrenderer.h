

#ifndef cellrenderer_h
#define cellrenderer_h

#include <pthread.h>
#include <sys/time.h>
#include "element.h"
#include "mtpipe.c"

typedef struct _cellrenderer_t cellrenderer_t;
struct _cellrenderer_t
{
    char      alive;
    pthread_t thread;

    input_t   input;
    mtpipe_t* elements_to_render;
};

cellrenderer_t* cellrenderer_alloc(font_t* font, char* respath);

void cellrenderer_queue(cellrenderer_t* renderer, element_t* element);

#endif
