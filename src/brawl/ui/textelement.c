

#include "textelement.h"
#include "mtmem.c"
#include <string.h>

void textdata_dealloc(void* pointer);
void textelement_updatetext(element_t* element, font_t* font, cmdqueue_t* cmdqueue);
void textelement_realdelete(element_t* element, font_t* font, cmdqueue_t* cmdqueue);
void textelement_input(element_t* element, input_t* input);

/* alloc text element */

element_t* textelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    mtstr_t*    string,
    mtstr_t*    prompt,
    font_t*     font,
    textstyle_t text)
{

    element_t*  element = element_alloc("text", name, x, y, width, height, NULL);
    textdata_t* data    = mtmem_calloc(sizeof(textdata_t), textdata_dealloc);

    data->style  = text;
    data->string = string == NULL ? mtstr_alloc() : mtmem_retain(string);
    data->prompt = prompt == NULL ? mtstr_alloc() : mtmem_retain(prompt);

    /* initial fillup */

    float scale = stbtt_ScaleForPixelHeight(&(font->info), text.textsize);
    float desc  = font->descent * scale;
    float asc   = font->ascent * scale;
    float max   = asc - desc;

    data->ascent       = asc;
    data->cursor.index = 0;
    data->selections   = mtvec_alloc();
    data->metrics      = mtmem_calloc(sizeof(glyphmetrics_t), NULL);

    element->input = textelement_input;

    if (text.editable == 1)
    {
	data->mask           = solidelement_alloc("mask", 0.0, 0.0, width, max, text.backcolor);
	data->cursor.element = solidelement_alloc("cursor", 0.0, 0.0, data->style.cursorsize, max, 0x000000FF);

	data->mask->translation           = &data->maskpos;
	data->cursor.element->translation = &data->cursor.curspos;
    }

    if (text.selectable == 1)
    {
	textselection_t* selection = mtmem_alloc(sizeof(textselection_t), NULL);
	selection->startindex      = 0;
	selection->endindex        = 0;
	selection->backcolor       = 0x000000FF;
	selection->textcolor       = 0xFFFFFFFF;

	data->selection = selection;
    }

    element_setdata(element, data);

    mtmem_release(data);

    textelement_updatetext(element, font, NULL);

    return element;
}

/* dealloc text element */

void textdata_dealloc(void* pointer)
{
    textdata_t* data = pointer;
    mtmem_release(data->string);
    mtmem_release(data->prompt);
    mtmem_release(data->metrics);
    mtmem_release(data->selections);
}

/* clear text element */

void textelement_clear(
    element_t*  element,
    font_t*     font,
    float       scale,
    cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    mtmem_release(data->string);
    data->string = mtstr_alloc();

    mtvec_reset(data->selections);

    data->cursor.index = 0;
    textelement_updatetext(element, font, cmdqueue);
}

/* resize text element */

void textelement_resize(element_t* element, float width, float height, font_t* font, cmdqueue_t* cmdqueue)
{
    textelement_updatetext(element, font, cmdqueue);
}

/* sets text */

void textelement_settext(element_t* element, font_t* font, cmdqueue_t* cmdqueue, mtstr_t* string)
{
    textdata_t* data = element->data;

    mtmem_release(data->string);

    data->string = mtstr_fromstring(string);

    mtvec_reset(data->selections);

    data->cursor.index = 0;
    textelement_updatetext(element, font, cmdqueue);
}

/* adds text */

void textelement_addtext(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    if (data->style.uppercase == 1)
    {
	for (int index = 0; index < strlen(input->stringa); index++)
	{
	    if (input->stringa[index] >= 97 && input->stringa[index] <= 122) input->stringa[index] -= 32;
	}
    }

    if (data->selection != NULL && data->selection->startindex != data->selection->endindex)
    {
	mtstr_t* string    = mtstr_frombytes(input->stringa);
	mtstr_t* newstring = mtstr_replace(data->string, string, data->selection->startindex, data->selection->endindex);
	data->cursor.index = data->selection->startindex + string->length;
	mtmem_releaseeach(string, data->string, NULL);
	data->string                = newstring;
	data->selection->startindex = data->selection->endindex;
    }
    else if (data->cursor.index < data->string->length - 1 && data->string->length > 0)
    {
	mtstr_t* string    = mtstr_frombytes(input->stringa);
	mtstr_t* newstring = mtstr_replace(data->string, string, data->cursor.index, data->cursor.index);
	data->cursor.index += string->length;
	mtmem_releaseeach(string, data->string, NULL);
	data->string = newstring;
    }
    else
    {
	mtstr_addbytearray(data->string, input->stringa);
	data->cursor.index = data->string->length;
    }

    textelement_updatetext(element, input->font, input->cmdqueue);
}

/* deletes last character */

void textelement_realdelete(element_t* element, font_t* font, cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    data->realdelcounter = 0;
    if (data->string->length > 0)
    {
	if (data->cursor.index < data->string->length - 1)
	{
	    mtstr_removecodepointatindex(data->string, data->cursor.index);
	}
	else mtstr_removecodepointatindex(data->string, data->string->length - 1);
    }

    textelement_updatetext(element, font, cmdqueue);
}

/* updates cursor */

void textelement_updatecursor(element_t* element)
{
    textdata_t* data = element->data;

    if (data->style.editable == 1)
    {
	if (data->cursor.index > -1 && data->cursor.index <= data->string->length)
	{
	    glyphmetrics_t* metrics  = &data->metrics[data->cursor.index];
	    data->cursor.targetpos.x = metrics->x + metrics->width;
	    data->cursor.targetpos.y = metrics->y - data->ascent;
	}
    }
}

/* updates text */

void textelement_updatetext(element_t* element, font_t* font, cmdqueue_t* cmdqueue)
{
    textdata_t* data = element->data;

    float oldwidth  = element->width;
    float oldheight = element->height;

    mtstr_t* string = data->string;

    if (data->string->length > 0)
    {
	mtstr_t* linktoken  = mtstr_frombytes("http://");
	mtstr_t* spacetoken = mtstr_frombytes(" ");
	uint32_t index      = mtstr_find(string, linktoken, 0);

	while (index < UINT32_MAX)
	{
	    uint32_t spaceindex = mtstr_find(string, spacetoken, index);
	    if (spaceindex == UINT32_MAX) spaceindex = data->string->length;

	    textselection_t* selection = mtmem_alloc(sizeof(textselection_t), NULL);
	    selection->startindex      = index;
	    selection->endindex        = spaceindex;
	    selection->textcolor       = 0x0000FFFF;
	    selection->backcolor       = 0xFFFF00FF;

	    mtvec_add(data->selections, selection);

	    index = mtstr_find(string, linktoken, index + 1);
	}

	mtmem_release(linktoken);
	mtmem_release(spacetoken);
    }

    if (data->string->length == 0 && element->focused == 0) string = data->prompt;

    data->metrics   = mtmem_realloc(data->metrics, sizeof(glyphmetrics_t) * (string->length + 2));
    mtbmp_t* bitmap = font_render_text(element->width + 1, element->height + 1, string, font, data->style, data->metrics, data->selections);

    if (bitmap != NULL)
    {
	element->width  = (float) bitmap->width - 1.0;
	element->height = (float) bitmap->height - 1.0;
    }

    mtmem_release(element->bitmap);

    if (element->width != oldwidth || element->height != oldheight)
    {
	char* onsizechange = mtmap_get(element->actions, "onsizechange");
	if (onsizechange != NULL && cmdqueue != NULL) cmdqueue_add(cmdqueue, onsizechange, element, NULL);
    }

    element->bitmap                    = bitmap;
    element->texture.coordstamp.tv_sec = 0;

    textelement_updatecursor(element);
}

/* returns border for glyph at coordinate */

float getborder_for_coordinate(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    float x = input->floata - element->finalx;
    // float y = input->floatb - element->finaly;

    for (int index = 0; index < data->string->length; index++)
    {
	glyphmetrics_t* metrics = &data->metrics[index + 1];
	if (x < metrics->x) return metrics->x;
    }
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
	glyphmetrics_t* metrics = &data->metrics[index + 1];
	if (x < metrics->x && y < metrics->y) return index;
    }
    return data->string->length;
}

/* key down event */

void textelement_keydown(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    if (data->style.editable == 0) return;

    char    key  = input->key;
    font_t* font = input->font;

    char* onchange = mtmap_get(element->actions, "onchange");

    if (input->key == kInputKeyTypeReturn)
    {
	char* onenter = mtmap_get(element->actions, "onenter");
	if (onenter != NULL) cmdqueue_add(input->cmdqueue, onenter, element, NULL);
    }
    else if (key == kInputKeyTypeBackspace)
    {
	data->cursor.index -= 1;
	textelement_updatecursor(element);
	textelement_realdelete(element, font, input->cmdqueue);
	// if ( data->realdelcounter > 0 )
	// data->realdelcounter = 1;
	input->upload = 1;
	if (onchange != NULL) cmdqueue_add(input->cmdqueue, onchange, element, NULL);
	return;
    }
    else if (key == kInputKeyTypeLeftArrow)
    {
	data->cursor.index -= 1;
	textelement_updatecursor(element);
	return;
    }
    else if (key == kInputKeyTypeRightArrow)
    {
	data->cursor.index += 1;
	textelement_updatecursor(element);
	return;
    }
    else
    {
	textelement_addtext(element, input);
	input->upload = 1;
	if (onchange != NULL) cmdqueue_add(input->cmdqueue, onchange, element, NULL);
    }
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
    textdata_t* data = element->data;

    element_input(element, input);

    if (data->style.selectable == 1)
    {
	data->selection->startindex = data->selection->endindex = getindex_for_coordinate(element, input);
	data->cursor.index                                      = getindex_for_coordinate(element, input);
	textelement_updatetext(element, input->font, input->cmdqueue);
    }

    int index = getindex_for_coordinate(element, input);

    if (data->dragged == 0 && data->metrics[index].selected == 1)
    {
	for (int sindex = 0; sindex < data->selections->length; sindex++)
	{
	    textselection_t* selection = data->selections->data[sindex];
	    if (index >= selection->startindex && index <= selection->endindex)
	    {
		element->notouchunder = 1;
		mtstr_t* sstring      = mtstr_substring(data->string, selection->startindex, selection->endindex);
		char*    cstring      = mtstr_bytes(sstring);
		cmdqueue_add(input->cmdqueue, "app.openlink", element, cstring);
		mtmem_release(cstring);
		mtmem_release(sstring);
	    }
	}
    }

    if (element->focused == 0)
    {
	if (data->style.editable == 1)
	{
	    cmdqueue_add(input->cmdqueue, "ui.setfocused", element, NULL);
	}
    }
    else
    {
	cmdqueue_add(input->cmdqueue, "view.showcopypaste", element, NULL);
    }
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
    textdata_t* data = element->data;

    element_input(element, input);

    data->dragged = 1;

    if (data->style.selectable == 1)
    {
	element->notouchunder = 1;
	uint32_t index        = getindex_for_coordinate(element, input);
	if (data->selection->startindex != index && data->selection->endindex != index)
	{
	    if (data->selection->startindex > index) data->selection->startindex = index;
	    else data->selection->endindex = index;
	    mtvec_adduniquedata(data->selections, data->selection);
	    textelement_updatetext(element, input->font, input->cmdqueue);
	}
    }

    input->upload = 1;
}

/* touch up event */

void textelement_touchup(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    element_input(element, input);
    input->upload = 1;

    if (data->style.selectable == 1)
    {
	if (data->selection->startindex == data->selection->endindex) element->notouchunder = 0;

	mtvec_remove(data->selections, data->selection);
    }

    // data->cursor.index = getindex_for_coordinate( element , input );
    // textelement_updatecursor( element );
}

/* resize event */

void textelement_resizeevent(element_t* element, input_t* input)
{
    element_input(element, input);
    textelement_resize(element, element->width, element->height, input->font, input->cmdqueue);
}

/* focus event */

void textelement_focus(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    element->focused = 1;
    textelement_updatetext(element, input->font, input->cmdqueue);
    if (data->style.editable == 1)
    {
	// element_addsubelement( element , data->mask );
	element_addsubelement(element, data->cursor.element);
	// element_settranslation( data->mask, &data->maskpos );
	element_settranslation(data->cursor.element, &data->cursor.curspos);
	data->cursor.index = data->string->length;
	textelement_updatecursor(element);
	input->upload = 1;
    }
    if (data->style.editable == 1)
    {
	cmdqueue_add(input->cmdqueue, "app.showkeyboard", element, NULL);
	cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);
    }
}

/* focus event */

void textelement_unfocus(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    if (data->style.editable == 1)
    {
	// element_removesubelement( element , data->mask );
	element_removesubelement(element, data->cursor.element);
	cmdqueue_add(input->cmdqueue, "ui.removeastimed", element, NULL);
	cmdqueue_add(input->cmdqueue, "view.hidecopypaste", element, NULL);
	cmdqueue_add(input->cmdqueue, "app.hidekeyboard", element, NULL);
	input->upload = 1;
    }
    element->focused = 0;
    textelement_updatetext(element, input->font, input->cmdqueue);
}

/* timer event, update cursor */

void textelement_timer(element_t* element, input_t* input)
{
    textdata_t* data = element->data;

    element_timer(element, input);

    if (data->style.editable == 1)
    {
	if (element->focused == 1)
	{
	    data->cursor.blinkcounter++;

	    data->cursor.actualpos.x += (data->cursor.targetpos.x - data->cursor.actualpos.x) / 4.0;
	    data->cursor.actualpos.y += (data->cursor.targetpos.y - data->cursor.actualpos.y) / 4.0;

	    /* shift mask if end of line */

	    data->maskpos = data->cursor.actualpos;

	    if (data->realdelcounter > 0)
	    {
		data->realdelcounter++;
		if (data->realdelcounter == 20) textelement_realdelete(element, input->font, input->cmdqueue);
		input->upload = 1;
	    }

	    if (data->cursor.blinkcounter < 20)
	    {
		data->cursor.curspos.x = data->cursor.actualpos.x;
		data->cursor.curspos.y = data->cursor.actualpos.y;
	    }
	    else if (data->cursor.blinkcounter < 40)
	    {
		data->cursor.curspos.x = -element->finalx * 2;
		data->cursor.curspos.y = -element->finaly * 2;
	    }
	    else
	    {
		data->cursor.blinkcounter = 0;
	    }

	    input->render = 1;
	}
    }
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
