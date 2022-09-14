

#ifndef list_h
#define list_h

#include "math2.c"

#define kListAlignTop 0
#define kListAlignBottom 1

typedef struct _listitem_t
{
    float position;
    float height;
    float zoomedheight;
} listitem_t;

typedef struct _list_t
{
    float translation; /* inner translation of list */
    float scrollratio; /* scroll ratio */

    float listsize;   /* list elements size / visible size */
    float itemsize;   /* the summed size of visible list elements, usually bigger than listsize */
    float minrowsize; /* minimum row size */

    float maxindex;  /* index of the last element/count of all available elements */
    float headindex; /* index of the topmost element */

    float zoom;      /* zoom ratio of list */
    float momentum;  /* scroll momentum of list */
    float autospeed; /* auto scroll speed */

    char align; /* aligment, top or bottom */

    char needshead; /* space for head is available */
    char needstail; /* space for tail is available */

    char removehead; /* head element is out of visible area */
    char removetail; /* tail element is out of visible area */

    uint32_t length;      /* items in list */
    uint32_t length_real; /* size of item list, must be bigger than length */

    listitem_t* items;

} list_t;

list_t* list_alloc(float height, char align);

void list_reset(list_t* list);

void list_setheight(list_t* list, float height);

void list_setheadindex(list_t* list, uint32_t index, uint32_t max);

void list_addhead(list_t* list, float height);

void list_addtail(list_t* list, float height);

void list_removehead(list_t* list, float height);

void list_removetail(list_t* list, float height);

void list_zoom(list_t* list, float delta);

void list_setzoom(list_t* list, float delta);

void list_scroll(list_t* list, float delta);

#endif
