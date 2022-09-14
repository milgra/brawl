

#ifndef textelement_h
#define textelement_h

#include "element.h"
#include "font.c"
#include "input.h"
#include "mtstr.c"
#include "text.c"
#include <stdio.h>

typedef struct _cursor_t
{
    element_t* element;
    int32_t    index;
    char       blinkcounter; /* counter for cursor blink */
    v2_t       targetpos;    /* cursor elements target position */
    v2_t       actualpos;    /* cusros elements actual advised position */
    v2_t       curspos;      /* cursor elements reference position */

} cursor_t;

typedef struct _textdata_t
{
    mtstr_t* string; /* visible string */
    mtstr_t* prompt; /* prompt string if normal string is not available */

    cursor_t         cursor;
    textstyle_t      style;     /* element's style */
    glyphmetrics_t*  metrics;   /* metrics of visible glyphs, 0 index is empty */
    textselection_t* selection; /* current selection of text element */

    mtvec_t* selections; /* special selections ( hyperlink , etc ) */

    float ascent; /* ascent of current textstyle */

    int  realdelcounter; /* counter for delete */
    char dragged;

    element_t* mask;
    v2_t       maskpos;

} textdata_t;

element_t* textelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    mtstr_t*    string,
    mtstr_t*    prompt,
    font_t*     font,
    textstyle_t text);

void textelement_clear(element_t* element, font_t* font, float scale, cmdqueue_t* cmdqueue);

void textelement_resize(element_t* element, float width, float height, font_t* font, cmdqueue_t* cmdqueue);

void textelement_settext(element_t* element, font_t* font, cmdqueue_t* cmdqueue, mtstr_t* string);

void textelement_addtext(element_t* element, input_t* input);

#endif
