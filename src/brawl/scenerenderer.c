
#ifndef scenerenderer_h
#define scenerenderer_h

#include "bus.c"
#include "floatbuffer.c"
#include "ogl.c"
#include "zc_mat4.c"
#include <string.h>

typedef struct _gamerenderdata_t
{

    char           layers[10];
    char           layer;
    m4_t           matrix;
    floatbuffer_t* buffer;

} gamerenderdata_t;

typedef struct _scenerenderer_t scenerenderer_t;
struct _scenerenderer_t
{
    GLuint shader;
    GLint  locations[2];
    GLint  slocations[2];
    m4_t   perspective_matrix;

    GLuint   vertexmodes[32];
    GLuint   vertexbuffers[32];
    uint32_t vertexbufferlengths[32];
    GLuint   vbo_main;
};

void scenerenderer_init(float width, float height);
void scenerenderer_free(void);
void scenerenderer_reset(void);
void scenerenderer_draw(void);
void scenerenderer_resize(float width, float height);
void scenerenderer_create_buffer(uint8_t layer, GLuint mode);
void scenerenderer_draw_unit_quad(void);

#endif /* scenerenderer_h */

#if __INCLUDE_LEVEL__ == 0

scenerenderer_t renderer;

#include "zc_memory.c"

#if defined(IOS) || defined(ASMJS) || defined(ANDROID)
const char* gvertex_point_source =
    #include "shaders/game_es.vsh"
    ;
const char* gfragment_point_source =
    #include "shaders/game_es.fsh"
    ;
#else
const char* gvertex_point_source =
    #include "shaders/game.vsh"
    ;
const char* gfragment_point_source =
    #include "shaders/game.fsh"
    ;
#endif

void scenerenderer_onmessage(const char* name, void* data);

/* alloc renderer */

void scenerenderer_init(float width, float height)
{

    bus_subscribe("RND", scenerenderer_onmessage);

    const char* uniforms[]   = {"1", "projection"};
    const char* attributes[] = {"2", "position", "color"};

    renderer.shader = ogl_shader_create(gvertex_point_source, gfragment_point_source, uniforms, attributes, renderer.locations);

    glUseProgram(renderer.shader);
    glClearColor(0.5, 0.5, 0.5, 1.0);

#ifdef OSX
    glPointSize(5.0);
#endif

    scenerenderer_reset();
    scenerenderer_resize(width, height);

    ogl_vertexbuffer(&renderer.vbo_main);
}

/* dealloc renderer */

void scenerenderer_free()
{

    ogl_shader_delete(renderer.shader);
}

/* resets renderer */

void scenerenderer_reset()
{
}

/* resizes renderer */

void scenerenderer_resize(float width, float height)
{

    glViewport(0.0, 0.0, width, height);

    m4_t pers_matrix = m4_defaultortho(0.0, width, -height, 0.0, 0.0, 1.0);
    m4_t tran_matrix = m4_defaulttranslation(-2 * width, height, 0.0);

    renderer.perspective_matrix = m4_multiply(pers_matrix, tran_matrix);

    // drawer->perspective_matrix = m4_multiply( m4_defaultscale( 1.0, 1, 0.5 ) , drawer->perspective_matrix );
}

/* creates a vertexbuffer for triangles on the given layer */

void scenerenderer_create_buffer(uint8_t layer, GLuint mode)
{

    GLuint name;

    glGenBuffers(1, &name);
    glBindBuffer(GL_ARRAY_BUFFER, name);

    renderer.vertexmodes[layer]   = mode;
    renderer.vertexbuffers[layer] = name;
}

/* updates a vertexbuffer for triangles */

void scenerenderer_update_buffer(uint8_t layer, floatbuffer_t* buffer)
{

    renderer.vertexbufferlengths[layer] = buffer->length;

    glBindBuffer(GL_ARRAY_BUFFER, renderer.vertexbuffers[layer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buffer->length, buffer->data, GL_DYNAMIC_DRAW);
}

/* renders a vertexbuffer for triangles */

void scenerenderer_render_triangles(uint8_t layer)
{

    glBindBuffer(GL_ARRAY_BUFFER, renderer.vertexbuffers[layer]);

    ogl_enableva_f2u1();

    matrix4array_t projection;

    projection.matrix = renderer.perspective_matrix;

    glUniformMatrix4fv(renderer.locations[0], 1, 0, projection.array);
    glDrawArrays(GL_TRIANGLES, 0, renderer.vertexbufferlengths[layer] / 3);
}

/* render points */

void scenerenderer_render_points(uint8_t layer)
{

    matrix4array_t projection;

    projection.matrix = renderer.perspective_matrix;

    glUniformMatrix4fv(renderer.locations[0], 1, 0, projection.array);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.vertexbuffers[layer]);

    ogl_enableva_f2u1();

    glDrawArrays(GL_POINTS, 0, renderer.vertexbufferlengths[layer] / 3);
}

/* render lines */

void scenerenderer_render_lines(uint8_t layer)
{

    matrix4array_t projection;

    projection.matrix = renderer.perspective_matrix;

    glUniformMatrix4fv(renderer.locations[0], 1, 0, projection.array);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.vertexbuffers[layer]);

    ogl_enableva_f2u1();

    glDrawArrays(GL_LINES, 0, renderer.vertexbufferlengths[layer] / 3);
}

/* render */

void scenerenderer_render(GLuint layer)
{

    GLuint mode = renderer.vertexmodes[layer];

    if (mode == GL_POINTS)
    {

	scenerenderer_render_points(layer);
    }
    else if (mode == GL_LINES)
    {

	scenerenderer_render_lines(layer);
    }
    else if (mode == GL_TRIANGLES)
    {

	scenerenderer_render_triangles(layer);
    }
}

/* message arrived */

void scenerenderer_onmessage(const char* name, void* data)
{

    if (strcmp(name, "UPDBUFF") == 0)
    {

	gamerenderdata_t* rdata = (gamerenderdata_t*) data;
	scenerenderer_update_buffer(rdata->layer, rdata->buffer);
    }
    else if (strcmp(name, "RNDBUFF") == 0)
    {

	gamerenderdata_t* rdata = (gamerenderdata_t*) data;

	glUseProgram(renderer.shader);

	renderer.perspective_matrix = rdata->matrix;

	for (int index = 0; index < 4; index++)
	{

	    uint32_t layer = rdata->layers[index];
	    scenerenderer_render(layer);
	}
    }
}

#endif
