

    /* Cell renderer is for background list cell/ui element content rendering to avoid lagging.  */

    #include "cellrenderer.h"


    void cellrenderer_dealloc( void* pointer );
	void cellrenderer_timerloop( cellrenderer_t* renderer );


    /* alloc cellrenderer */

    cellrenderer_t* cellrenderer_alloc( font_t* font , char* respath )
    {
        cellrenderer_t* result = mtmem_calloc( sizeof( cellrenderer_t ), cellrenderer_dealloc );
        result->alive = 1;
        result->elements_to_render = mtpipe_alloc( 100 );

        result->input.font = mtmem_retain( font );;
        result->input.type = kInputTypeRender;
        result->input.respath = respath;

        #ifndef ASMJS
        pthread_create(&result->thread, NULL, (void*)cellrenderer_timerloop , result );
        mtmem_retain( result );
        #endif

        return result;
    }

    /* dealloc cellrenderer */

    void cellrenderer_dealloc( void* pointer )
    {
        cellrenderer_t* renderer = pointer;
        mtmem_release( renderer->elements_to_render );
        mtmem_release( renderer->input.font );
    }

    /* timer thread */

	void cellrenderer_timerloop( cellrenderer_t* renderer )
	{
        while ( renderer->alive )
        {
            element_t* element = mtpipe_recv( renderer->elements_to_render );

            if ( element != NULL )
            {
                element->input( element , &renderer->input );
                mtmem_release( element );
            }
            else
            {
                #ifdef RASPBERRY
                usleep( 100 );
                #else
                struct timespec time;
                time.tv_sec = 0;
                time.tv_nsec = 100000000L;
                nanosleep(&time , (struct timespec *)NULL);
                #endif
            }
        }

        mtmem_release( renderer );
	}

    /* queue element for render */

    void cellrenderer_queue( cellrenderer_t* renderer , element_t* element )
    {
        mtmem_retain( element );
        mtpipe_send( renderer->elements_to_render , element );
    }

    /* stops renderer and releases it after */

    void cellrenderer_stop_and_release( cellrenderer_t* renderer )
    {
        renderer->alive = 0;
    }
