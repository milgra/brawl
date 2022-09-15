#ifndef text_element_h
#define text_element_h

#include "element.c"
#include "input.h"
#include "zc_string.c"
#include "zc_text.c"
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
    str_t* string; /* visible string */
    str_t* prompt; /* prompt string if normal string is not available */

    cursor_t    cursor;
    textstyle_t style; /* element's style */

    vec_t* selections; /* special selections ( hyperlink , etc ) */

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
    str_t*      string,
    str_t*      prompt,
    textstyle_t text);

void textelement_clear(element_t* element, float scale, cmdqueue_t* cmdqueue);

void textelement_resize(element_t* element, float width, float height, cmdqueue_t* cmdqueue);

void textelement_settext(element_t* element, cmdqueue_t* cmdqueue, str_t* string);

void textelement_addtext(element_t* element, input_t* input);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "str_util.c"
#include "zc_memory.c"
#include <string.h>

void textdata_dealloc(void* pointer);
void textelement_updatetext(element_t* element, cmdqueue_t* cmdqueue);
void textelement_realdelete(element_t* element, cmdqueue_t* cmdqueue);
void textelement_input(element_t* element, input_t* input);

/* alloc text element */

element_t* textelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    str_t*      string,
    str_t*      prompt,
    textstyle_t text)
{
    element_t*  element = element_alloc("text", name, x, y, width, height, NULL);
    textdata_t* data    = CAL(sizeof(textdata_t), textdata_dealloc, NULL);

    data->style  = text;
    data->string = string == NULL ? str_new() : RET(string);
    data->prompt = prompt == NULL ? str_new() : RET(prompt);

    /* initial fillup */

    data->ascent       = 0;
    data->cursor.index = 0;
    data->selections   = VNEW();

    element->input = textelement_input;

    element_setdata(element, data);

    REL(data);

    textelement_updatetext(element, NULL);

    return element;
}

/* dealloc text element */

void textdata_dealloc(void* pointer)
{
    textdata_t* data = pointer;
    REL(data->string);
    REL(data->prompt);
    REL(data->selections);
}

/* clear text element */

void textelement_clear(
    element_t*  element,
    float       scale,
    cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    REL(data->string);
    data->string = str_new();

    vec_reset(data->selections);

    data->cursor.index = 0;
    textelement_updatetext(element, cmdqueue);
}

/* resize text element */

void textelement_resize(element_t* element, float width, float height, cmdqueue_t* cmdqueue)
{
    textelement_updatetext(element, cmdqueue);
}

/* sets text */

void textelement_settext(element_t* element, cmdqueue_t* cmdqueue, str_t* string)
{
    textdata_t* data = element->data;

    REL(data->string);

    data->string = str_new();
    str_add_string(data->string, string);

    vec_reset(data->selections);

    data->cursor.index = 0;
    textelement_updatetext(element, cmdqueue);
}

/* adds text */

void textelement_addtext(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    str_add_bytearray(data->string, input->stringa);
    data->cursor.index = data->string->length;

    textelement_updatetext(element, input->cmdqueue);
}

/* deletes last character */

void textelement_realdelete(element_t* element, cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    data->realdelcounter = 0;
    if (data->string->length > 0)
    {
	if (data->cursor.index < data->string->length - 1)
	{
	    str_remove_codepoint_at_index(data->string, data->cursor.index);
	}
	else str_remove_codepoint_at_index(data->string, data->string->length - 1);
    }

    textelement_updatetext(element, cmdqueue);
}

/* updates cursor */

void textelement_updatecursor(element_t* element)
{
}

/* updates text */

void textelement_updatetext(element_t* element, cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    float oldwidth  = element->width;
    float oldheight = element->height;

    str_t* string = data->string;

    if (data->string->length == 0 && element->focused == 0) string = data->prompt;

    /* data->metrics = mem_realloc(data->metrics, sizeof(glyphmetrics_t) * (string->length + 2)); */

    /* bm_rgba_t* bitmap = font_render_text(element->width + 1, element->height + 1, string, font, data->style, data->metrics, data->selections); */

    bm_rgba_t* bitmap = bm_rgba_new(element->width + 1, element->height + 1);
    text_render(string, data->style, bitmap);

    if (bitmap != NULL)
    {
	element->width  = (float) bitmap->w - 1.0;
	element->height = (float) bitmap->h - 1.0;
    }

    if (element->bitmap) REL(element->bitmap);

    if (element->width != oldwidth || element->height != oldheight)
    {
	char* onsizechange = MGET(element->actions, "onsizechange");
	if (onsizechange != NULL && cmdqueue != NULL) cmdqueue_add(cmdqueue, onsizechange, element, NULL);
    }

    element->bitmap                    = bitmap;
    element->texture.coordstamp.tv_sec = 0;

    textelement_updatecursor(element);
}

/* returns border for glyph at coordinate */

float getborder_for_coordinate(element_t* element, input_t* input)
{
    return 0.0;
}

/* returns index for glyph at coordinate */

int getindex_for_coordinate(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    float x = input->floata - element->finalx;
    float y = input->floatb - element->finaly;

    if (element->translation != NULL)
    {
	x -= element->translation->x;
	y -= element->translation->y;
    }

    for (int index = 0; index < data->string->length; index++)
    {
	/* glyphmetrics_t* metrics = &data->metrics[index + 1]; */
	/* if (x < metrics->x && y < metrics->y) return index; */
    }
    return data->string->length;
}

/* key down event */

void textelement_keydown(element_t* element, input_t* input)
{
}

/* key press event */

void textelement_keypress(element_t* element, input_t* input)
{
    if (element->focused == 0) return;
    textelement_keydown(element, input);
}

/* touch down event */

void textelement_touchdown(element_t* element, input_t* input)
{
    element_input(element, input);
}

/* touch down outside event */

void textelement_touchdownoutside(element_t* element, input_t* input)
{
    float x = input->floata - element->finalx;
    float y = input->floatb - element->finaly;

    if (y < -30.0 * input->scale || y > element->height || x < 0 || x > element->width)
    {
	cmdqueue_add(input->cmdqueue, "ui.setfocused", NULL, NULL);
	cmdqueue_add(input->cmdqueue, "view.hidecopypaste", element, NULL);
    }
}

/* touch drag event */

void textelement_touchdrag(element_t* element, input_t* input)
{
    element_input(element, input);
}

/* touch up event */

void textelement_touchup(element_t* element, input_t* input)
{
    element_input(element, input);
    input->upload = 1;

    // data->cursor.index = getindex_for_coordinate( element , input );
    // textelement_updatecursor( element );
}

/* resize event */

void textelement_resizeevent(element_t* element, input_t* input)
{
    element_input(element, input);
    textelement_resize(element, element->width, element->height, input->cmdqueue);
}

/* focus event */

void textelement_focus(element_t* element, input_t* input)
{
    element->focused = 1;
    textelement_updatetext(element, input->cmdqueue);
}

/* focus event */

void textelement_unfocus(element_t* element, input_t* input)
{
    element->focused = 0;
    textelement_updatetext(element, input->cmdqueue);
}

/* timer event, update cursor */

void textelement_timer(element_t* element, input_t* input)
{
    element_timer(element, input);
}

/* input event */

void textelement_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeTimer: textelement_timer(element, input); break;
	case kInputTypeTouchDown: textelement_touchdown(element, input); break;
	case kInputTypeTouchDownOutside: textelement_touchdownoutside(element, input); break;
	case kInputTypeTouchDrag: textelement_touchdrag(element, input); break;
	case kInputTypeTouchUp: textelement_touchup(element, input); break;
	case kInputTypeKeyPress: textelement_keydown(element, input); break;
	case kInputTypeResize: textelement_resizeevent(element, input); break;
	case kInputTypeFocus: textelement_focus(element, input); break;
	case kInputTypeUnFocus: textelement_unfocus(element, input); break;
	default: element_input(element, input); break;
    }
}

#endif
