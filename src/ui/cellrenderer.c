#ifndef cellrenderer_h
#define cellrenderer_h

#include "element.c"
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

cellrenderer_t* cellrenderer_alloc(char* respath);

void cellrenderer_queue(cellrenderer_t* renderer, element_t* element);

#endif

#if __INCLUDE_LEVEL__ == 0

/* Cell renderer is for background list cell/ui element content rendering to avoid lagging.  */

void cellrenderer_dealloc(void* pointer);
void cellrenderer_timerloop(cellrenderer_t* renderer);

/* alloc cellrenderer */

cellrenderer_t* cellrenderer_alloc(char* respath)
{
    cellrenderer_t* result     = CAL(sizeof(cellrenderer_t), cellrenderer_dealloc, NULL);
    result->alive              = 1;
    result->elements_to_render = ch_new(100);

    result->input.type    = kInputTypeRender;
    result->input.respath = respath;

#ifndef ASMJS
    pthread_create(&result->thread, NULL, (void*) cellrenderer_timerloop, result);
    RET(result);
#endif

    return result;
}

/* dealloc cellrenderer */

void cellrenderer_dealloc(void* pointer)
{
    cellrenderer_t* renderer = pointer;
    REL(renderer->elements_to_render);
}

/* timer thread */

void cellrenderer_timerloop(cellrenderer_t* renderer)
{
    while (renderer->alive)
    {
	element_t* element = ch_recv(renderer->elements_to_render);

	if (element != NULL)
	{
	    element->input(element, &renderer->input);
	    REL(element);
	}
	else
	{
	    struct timespec time;
	    time.tv_sec  = 0;
	    time.tv_nsec = 100000000L;
	    nanosleep(&time, (struct timespec*) NULL);
	}
    }

    REL(renderer);
}

/* queue element for render */

void cellrenderer_queue(cellrenderer_t* renderer, element_t* element)
{
    RET(element);
    ch_send(renderer->elements_to_render, element);
}

/* stops renderer and releases it after */

void cellrenderer_stop_and_release(cellrenderer_t* renderer)
{
    renderer->alive = 0;
}

#endif
