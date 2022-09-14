

#ifndef ui_h
#define ui_h

#include "element.h"
#include "zc_util2.c"
#include "zc_vector.c"

typedef struct _uitouch_t
{
    char   swiped;
    char*  id;
    vec_t* views;
    v2_t   start;
    v2_t   actual;
    v2_t   last;

} uitouch_t;

uitouch_t* uitouch_alloc(char* id, v2_t start);

typedef struct _ui_t
{
    map_t* touches;
    vec_t* timed;
    vec_t* visible;
    vec_t* focused;
    vec_t* zoomed;
    vec_t* scrolled;

    float swipedist;

} ui_t;

ui_t* ui_alloc(float swipedist);
void  ui_setfocused(ui_t* ui, element_t* element, input_t* input);

void ui_addtimed(ui_t* ui, element_t* element);
void ui_removetimed(ui_t* ui, element_t* element);

void ui_touch_down(ui_t* ui, input_t* input);
void ui_touch_drag(ui_t* ui, input_t* input);
void ui_touch_up(ui_t* ui, input_t* input);

void ui_zoom_start(ui_t* ui, input_t* input);
void ui_zoom(ui_t* ui, input_t* input);
void ui_zoom_end(ui_t* ui, input_t* input);

void ui_scroll_start(ui_t* ui, input_t* input);
void ui_scroll(ui_t* ui, input_t* input);
void ui_scroll_end(ui_t* ui, input_t* input);

void ui_keypress(ui_t* ui, input_t* input);
void ui_timer(ui_t* ui, input_t* input);

#endif
