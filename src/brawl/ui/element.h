

#ifndef element_h
#define element_h

#include "input.h"
#include "mtbmp.c"
#include "zc_map.c"
#include "zc_util2.c"
#include "zc_vector.c"
#include <stdio.h>
#include <sys/time.h>

typedef struct _element_animation_t
{
    int   delay;  /* animation should wait this much before start */
    char* action; /* this action is called at the end of the animation */
    v2_t  trans;  /* animation translation change */
    v2_t  target; /* animation translation target */
} element_animation_t;

typedef struct _element_texture_t
{
    v2_t ulc; /* upper left texture coordinate */
    v2_t urc; /* upper right texture coordinate */
    v2_t llc; /* lower left texture coordinate */
    v2_t lrc; /* lower right texture coordinate */

    char     ready;   /* texture is ready */
    char     tiled;   /* needs a tiled texture */
    uint32_t name;    /* opengl texture */
    uint8_t  sampler; /* texture sampler */

    struct timeval coordstamp; /* texture upload timestamp */

} element_texture_t;

typedef struct _element_autosize_t
{
    float fillx; /* horizontal percentage */
    float filly; /* vertical percentage */

    char keepxcenter; /* stay in x center */
    char keepycenter; /* stay in y center */

    float topmargin;    /* top margin distance */
    float bottommargin; /* bottom margin distance */
    float leftmargin;   /* left margin distance */
    float rightmargin;  /* right margin distance */
} element_autosize_t;

typedef struct _element_t element_t;
struct _element_t
{
    char focused;      /* element is focused */
    char exclude;      /* exclude element from drawing */
    char notouchunder; /* elements under self cannot get touch events */

    char* type;
    char* name;
    void* data;

    float x;
    float y;

    float width;
    float height;

    float finalx; /* absolute x position */
    float finaly; /* absolute y position */

    mtbmp_t* bitmap;      /* element's bitmap that will be used in the opengl texture map */
    map_t*   actions;     /* element actions */
    vec_t*   subelements; /* subelements */
    v2_t*    translation; /* translation, the address is used by the renderer to group elements for rendering */

    element_texture_t   texture;   /* texture information */
    element_autosize_t  autosize;  /* autosize information */
    element_animation_t animation; /* animation information */

    void (*input)(element_t*, input_t*);
};

element_t* element_alloc(char* type, char* name, float x, float y, float width, float height, mtbmp_t* bitmap);

void element_settype(element_t* element, char* type);

void element_setdata(element_t* element, void* data);

void element_setbitmap(element_t* element, mtbmp_t* bitmap);

void element_setaction(element_t* element, char* actionid, char* action);

void element_setfocused(element_t* element, char flag);

void element_setposition(element_t* element, float x, float y);

void element_setdimension(element_t* element, float width, float height);

void element_settranslation(element_t* element, v2_t* translation);

void element_addsubelement(element_t* element, element_t* subelement);

void element_addsubelementatindex(element_t* element, element_t* subelement, size_t index);

void element_removesubelement(element_t* element, element_t* subelement);

void element_collectelements(element_t* element, v2_t parent, vec_t* vector);

void element_animateto(element_t* element, float x, float y, int delay, char* action);

void element_timer(element_t* element, input_t* input);

void element_input(element_t* element, input_t* input);

element_t* solidelement_alloc(
    char*    name,
    float    x,
    float    y,
    float    width,
    float    height,
    uint32_t color);

void solidelement_setcolor(element_t* element, uint32_t color);

element_t* imageelement_alloc(
    char* name,
    float x,
    float y,
    float width,
    float height,
    char* imagepath);

void imageelement_resize(element_t* element, float width, float height);

element_t* videoelement_alloc(
    char*  name,
    float  x,
    float  y,
    float  width,
    float  height,
    str_t* path);

void videoelement_resize(element_t* element, float width, float height);

#endif
