

/* Cell renderer is for background list cell/ui element content rendering to avoid lagging.  */

#include "cellrenderer.h"

void cellrenderer_dealloc(void* pointer);
void cellrenderer_timerloop(cellrenderer_t* renderer);

/* alloc cellrenderer */

cellrenderer_t* cellrenderer_alloc(font_t* font, char* respath)
{
    cellrenderer_t* result     = CAL(sizeof(cellrenderer_t), cellrenderer_dealloc, NULL);
    result->alive              = 1;
    result->elements_to_render = ch_new(100);

    result->input.font = RET(font);
    ;
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
    REL(renderer->input.font);
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
