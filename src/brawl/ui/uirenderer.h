

#ifndef uirenderer_h
#define uirenderer_h

#include <sys/time.h>
#include "element.h"
#include "math2.c"
#include "ogl.c"
#include "floatbuffer.c"
#include "mtbmp.c"

typedef struct _texture_t
{
    int bottompos;
    int sidepos;

    int maxwidth;
    int maxsize;

    GLuint name;

} texture_t;

typedef struct _vertexgroup_t
{
    v2_t*    translation;
    uint32_t border;
    uint32_t start;
    uint8_t  sampler;
    GLint    texture;

} vertexgroup_t;

typedef struct _uirenderer_t
{
    GLuint shader;
    GLint  locations[6];
    GLuint vertexbuffer;

    texture_t textures[6];
    m4_t      perspective_matrix;

    int            texindex;
    struct timeval texreset;

    floatbuffer_t* buffer;

    char       needsupload;
    GLsizeiptr buffersize;

    /* group handling */

    vertexgroup_t actualgroup;
    vertexgroup_t groups[50];
    int32_t       groupcount;

    /* performance counters */

    long    lastcheck;
    int32_t drawcount;
    int32_t uploadsize;
    int32_t uploadcount;
    int32_t texturecount;

} uirenderer_t;

uirenderer_t* uirenderer_alloc(float width, float height);

void uirenderer_reset(uirenderer_t* renderer);

void uirenderer_resize(uirenderer_t* renderer, float width, float height);

char uirenderer_addelement(uirenderer_t* renderer, element_t* element);

void uirenderer_draw(uirenderer_t* renderer);

#endif
