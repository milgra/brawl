

	#include "uirenderer.h"


	const char* vertex_point_source =
	#if defined(IOS) || defined(ANDROID) || defined(ASMJS)
		"attribute vec4 position;\n"
		"attribute vec2 texcoord;\n"
		"varying highp vec2 texcoordv;\n"
		"uniform mat4 projection;\n"
		"void main ( )\n"
		"{\n"
			"gl_Position = projection * position;\n"
			"texcoordv = texcoord;"
		"}\n";
	#else
		"attribute vec4 position;\n"
		"attribute vec2 texcoord;\n"
		"varying vec2 texcoordv;\n"
		"uniform mat4 projection;\n"
		"void main ( )\n"
		"{\n"
			"gl_Position = projection * position;\n"
			"texcoordv = texcoord;\n"
		"}\n";
	#endif

	const char* fragment_point_source =
	#if defined(IOS) || defined(ASMJS)
		"varying highp vec2 texcoordv;\n"
		"uniform sampler2D texture_main;\n"
		"void main( )\n"
		"{\n"
		"	gl_FragColor = texture2D( texture_main , texcoordv , 0.0 );\n"
		"}\n";
    #elif defined(ANDROID)
            "#extension GL_OES_EGL_image_external : require\n"
            "varying highp vec2 texcoordv;\n"
            "uniform int texindex;\n"
            "uniform samplerExternalOES texture_video;\n"
            "uniform sampler2D texture_main;\n"
            "void main( )\n"
            "{\n"
            "   if ( texindex == 1 )\n"
            "   {\n"
            "       gl_FragColor = texture2D( texture_video, texcoordv );\n"
            "   }\n"
            "	else\n"
            "   {\n"
            "       gl_FragColor = texture2D( texture_main, texcoordv , 0.0 );\n"
            "   }\n"
            "}\n";
    #else
		"varying vec2 texcoordv;\n"
        "uniform int texindex;\n"
        "uniform sampler2D texture_video;\n"
		"uniform sampler2D texture_main;\n"
		"void main( )\n"
            "{\n"
            "   if ( texindex == 1 )\n"
            "   {\n"
//            "       gl_FragColor = texture2D( texture_video, texcoordv , 0.0 );\n"
            "   }\n"
            "	else\n"
            "   {\n"
            "       gl_FragColor = texture2D( texture_main, texcoordv , 0.0 );\n"
            "   }\n"
            "}\n";
	#endif


	/* create renderer */

    uirenderer_t* uirenderer_alloc( float width , float height )
	{
        char* uniforms[ ]   = { "4" , "projection" , "texture_main", "texture_video", "texindex" };
        char* attributes[ ] = { "2" , "position" , "texcoord" };

        uirenderer_t* uirenderer = mtmem_calloc( sizeof( uirenderer_t ) , NULL );

        uirenderer->shader = ogl_shader_create( vertex_point_source , fragment_point_source , ( const char** ) uniforms , ( const char** ) attributes , uirenderer->locations );
		uirenderer->buffer = floatbuffer_alloc( );

        gettimeofday( &uirenderer->texreset, NULL);

		glUseProgram( uirenderer->shader );
        glEnable( GL_TEXTURE_2D );
		glClearColor( 0.5,0.5,0.5,1.0);
        glGenBuffers( 1 , &uirenderer->vertexbuffer );
        glBindBuffer( GL_ARRAY_BUFFER , uirenderer->vertexbuffer );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

        ogl_enableva_f2f2();

        int max_size;
        glGetIntegerv( GL_MAX_TEXTURE_SIZE , &max_size );
        printf( "Max texture size is : %i\n" , max_size );
        if ( max_size > 4096 ) max_size = 4096;

        uirenderer->textures[ 0 ].maxsize = max_size;
        uirenderer->textures[ 1 ].maxsize = max_size;
        uirenderer->textures[ 2 ].maxsize = max_size;
        uirenderer->textures[ 3 ].maxsize = max_size;
        uirenderer->textures[ 4 ].maxsize = max_size;
        uirenderer->textures[ 5 ].maxsize = max_size;

        ogl_texture_create_nearest( &uirenderer->textures[0].name, max_size, max_size);
        ogl_texture_create_nearest( &uirenderer->textures[1].name, max_size, max_size);
        ogl_texture_create_nearest( &uirenderer->textures[2].name, max_size, max_size);
        ogl_texture_create_nearest( &uirenderer->textures[3].name, max_size, max_size);
        ogl_texture_create_nearest( &uirenderer->textures[4].name, max_size, max_size);
        ogl_texture_create_nearest( &uirenderer->textures[5].name, max_size, max_size);

		uirenderer_reset( uirenderer );
		uirenderer_resize( uirenderer , width , height );

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uirenderer->locations[1], 0);   // set main texture az TEXTURE0
        glActiveTexture(GL_TEXTURE1);
        glUniform1i(uirenderer->locations[2], 1);   // set main texture az TEXTURE1
        glUniform1i(uirenderer->locations[3], 0);   // set texindex to 0

		return uirenderer;
	}

	/* delete renderer */

	void uirenderer_dealloc( void* pointer )
	{
		uirenderer_t* renderer = pointer;

		ogl_shader_delete( renderer->shader );
		glDeleteBuffers( 1 , &renderer->vertexbuffer );
		mtmem_release( renderer->buffer );
        glDeleteTextures(1, &renderer->textures[0].name );
        glDeleteTextures(1, &renderer->textures[1].name );
        glDeleteTextures(1, &renderer->textures[2].name );
        glDeleteTextures(1, &renderer->textures[3].name );
        glDeleteTextures(1, &renderer->textures[4].name );
        glDeleteTextures(1, &renderer->textures[5].name );
	}

	/* resets renderer */

	void uirenderer_reset( uirenderer_t* renderer )
	{
		floatbuffer_reset( renderer->buffer );

		renderer->needsupload = 1;
        renderer->groupcount = -1;
        renderer->actualgroup.texture = -1;
        renderer->actualgroup.translation = NULL;
	}

	/* resizes renderer */

	void uirenderer_resize( uirenderer_t* renderer , float width , float height )
	{
        glViewport( 0.0 , 0.0 , width , height );
	}

    texture_t* uirenderer_gettextureobj( uirenderer_t* renderer , element_t* element )
    {
        texture_t* texobj = &renderer->textures[renderer->texindex];

        if ( texobj->bottompos + element->bitmap->height >= texobj->maxsize )
        {
            texobj->bottompos = 0;
            texobj->sidepos += texobj->maxwidth;
            texobj->maxwidth = 0;
        }

        if ( texobj->sidepos + element->bitmap->width >= texobj->maxsize )
        {
            texobj->bottompos = 0;
            texobj->sidepos = 0;
            texobj->maxwidth = 0;

            renderer->texindex += 1;

            if ( renderer->texindex == 6 )
            {
                renderer->texindex = 0;
                gettimeofday( &renderer->texreset, NULL);
                return NULL;
            }

            texobj = &renderer->textures[renderer->texindex];
        }

        return texobj;
    }

	/* adds element */

	char uirenderer_addelement( uirenderer_t* renderer , element_t* element )
	{
		floatbuffer_t* buffer = renderer->buffer;

        /* update tiled element's texture data if needed */

        if ( element->texture.tiled == 1 )
        {
            if ( element->bitmap != NULL )
            {
                if ( element->texture.coordstamp.tv_sec  != renderer->texreset.tv_sec ||
                     element->texture.coordstamp.tv_usec != renderer->texreset.tv_usec )
                {
                    texture_t* texobj = uirenderer_gettextureobj( renderer , element );
                    if ( texobj == NULL ) return -1;

                    element->texture.name = texobj->name;

                    /* upload bitmap as tile in texture */

                    glBindTexture( GL_TEXTURE_2D , texobj->name );
                    glTexSubImage2D( GL_TEXTURE_2D , 0 , texobj->sidepos , texobj->bottompos , element->bitmap->width , element->bitmap->height , GL_RGBA , GL_UNSIGNED_BYTE , element->bitmap->bytes );

                    renderer->texturecount ++;
                    if ( element->bitmap->width > texobj->maxwidth ) texobj->maxwidth = element->bitmap->width;

                    int wth = element->bitmap->width;
                    int hth = element->bitmap->height;

                    /* this is for avoiding texel artifacts */

                    float ulx = ( (float)texobj->sidepos + 0.5 ) / ( ( float ) texobj->maxsize );
                    float uly = ( (float)texobj->bottompos + 0.5 ) / ( ( float ) texobj->maxsize );
                    float urx = ( (float)texobj->sidepos + (float)wth - 0.5 ) / ( ( float ) texobj->maxsize );
                    float ury = uly;
                    float llx = ulx;
                    float lly = ( (float)texobj->bottompos + (float)hth - 0.5) / ( ( float ) texobj->maxsize );
                    float lrx = urx;
                    float lry = lly;

                    element->texture.ulc = v2_init( ulx , uly );
                    element->texture.urc = v2_init( urx , ury );
                    element->texture.llc = v2_init( llx , lly );
                    element->texture.lrc = v2_init( lrx , lry );

                    texobj->bottompos += element->bitmap->height;

                    element->texture.coordstamp = renderer->texreset;
                }
            }
        }
        else
        {
            if ( element->texture.name == 0 ) ogl_texture_create_linear( &element->texture.name, 512, 512 );
        }

		/* create new group if needed */

		if ( renderer->actualgroup.translation != element->translation || renderer->actualgroup.texture != element->texture.name )
		{
            renderer->groupcount += 1;

            vertexgroup_t group =
            {
                .translation = element->translation ,
                .texture     = element->texture.name ,
                .sampler     = element->texture.sampler ,
                .border      = buffer->length ,
                .start       = buffer->length ,
            };

            renderer->actualgroup = group;
            renderer->groups[ renderer->groupcount ] = group;
		}

        // printf( "ADDING ELEMENT %s x %f y %f w %f h %f tx %f ty %f\n" , element->name , element->finalx , element->finaly , element->width , element->height , element->translation == NULL ? 0.0 : element->translation->x , element->translation == NULL ? 0.0 : element->translation->y );

        /* add vertex data to float buffer */

		floatbuffer_addvector2( buffer , v2_init( element->finalx , element->finaly ) );
		floatbuffer_addvector2( buffer , element->texture.ulc );
		floatbuffer_addvector2( buffer , v2_init( element->finalx , element->finaly + element->height ) );
		floatbuffer_addvector2( buffer , element->texture.llc );
		floatbuffer_addvector2( buffer , v2_init( element->finalx + element->width , element->finaly + element->height ) );
		floatbuffer_addvector2( buffer , element->texture.lrc );

		floatbuffer_addvector2( buffer , v2_init( element->finalx , element->finaly ) );
		floatbuffer_addvector2( buffer , element->texture.ulc );
		floatbuffer_addvector2( buffer , v2_init( element->finalx + element->width , element->finaly + element->height ) );
		floatbuffer_addvector2( buffer , element->texture.lrc );
		floatbuffer_addvector2( buffer , v2_init( element->finalx + element->width , element->finaly ) );
		floatbuffer_addvector2( buffer , element->texture.urc );

		/* update last group border */

		renderer->groups[ renderer->groupcount ].border = buffer->length;

        return 0;
	}

	/* draw groups */

	void uirenderer_draw( uirenderer_t* renderer )
	{
        glBindBuffer( GL_ARRAY_BUFFER , renderer->vertexbuffer );
        ogl_enableva_f2f2();

        glUseProgram( renderer->shader );

//        struct timeval now;
//        gettimeofday( &now , NULL );
//        if ( now.tv_sec - renderer->lastcheck > 0 )
//        {
//            printf( "DRAW COUNT %i UPLOAD COUNT %i TEXTURE COUNT %i UPLOAD SIZE %i\n" , renderer->drawcount , renderer->uploadcount , renderer->texturecount , renderer->uploadsize );
//            renderer->drawcount = 0;
//            renderer->uploadsize = 0;
//            renderer->uploadcount = 0;
//            renderer->texturecount = 0;
//            renderer->lastcheck = now.tv_sec;
//        }
//        renderer->drawcount++;

        /* upload vertex data if needed */

		if ( renderer->needsupload == 1 )
		{
            GLsizeiptr size = sizeof(GLfloat) * renderer->buffer->length;
            if ( size > renderer->buffersize )
            {
                glBufferData( GL_ARRAY_BUFFER , size , renderer->buffer->data , GL_DYNAMIC_DRAW );
                renderer->buffersize = size;
            }
            else
            {
                glBufferSubData( GL_ARRAY_BUFFER , 0 , size , renderer->buffer->data );
            }
			renderer->needsupload = 0;
//            renderer->uploadcount ++;
//            renderer->uploadsize += size;
		}

        /* draw groups */

		for ( int index = 0 ; index < renderer->groupcount + 1; index++ )
		{
			matrix4array_t projection;
			vertexgroup_t* group = &renderer->groups[ index ];
			v2_t* translation = group->translation;

            /* swith video and main texture if needed ( android ) */

            if ( group->sampler == 0 )
            {
                glActiveTexture( GL_TEXTURE0 );
                glUniform1i(renderer->locations[3], 0);
            }
            else
            {
                glActiveTexture( GL_TEXTURE1 );
                glUniform1i(renderer->locations[3], 1);
            }

            glBindTexture( GL_TEXTURE_2D , group->texture );

			if ( translation != NULL ) projection.matrix = m4_translate( renderer->perspective_matrix , translation->x , translation->y , 0.0 );
			else projection.matrix = renderer->perspective_matrix;

			glUniformMatrix4fv( renderer->locations[ 0 ] , 1 , 0 , projection.array );
			glDrawArrays( GL_TRIANGLES , group->start / 4 , ( group->border - group->start ) / 4 );
		}
	}
