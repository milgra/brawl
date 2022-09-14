#ifndef listelement_h
#define listelement_h

#include "element.c"
#include "input.h"
#include "list.c"
#include "textelement.c"
#include "zc_vector.c"

typedef struct _listdata_t
{
    list_t* list; /* abstract list loginc */

    vec_t* items;     /* list items */
    vec_t* headqueue; /* elements waiting for to be added to head */
    vec_t* tailqueue; /* elements waiting for to be added to tail */

    element_t* scrollbar; /* scrollbar */

    char bounce;      /* list should bounce */
    char firepull;    /* pull event should be fired */
    char buffersize;  /* maximum element number in head/tail buffers */
    char newatbottom; /* new elements should be at bottom */

    char dragging;  /* indicates that list is being dragged */
    char scrolling; /* indicates that list is being scrolled */
    char rendering; /* indicates if list elements are being renderer in a different thread */

    float zoom; /* zoom factor */
    float momentum;
    float autospeed;
    float lasttouch;

    v2_t translation;       /* translation of elements */
    v2_t scrolltranslation; /* translation of scrollbar */

} listdata_t;

element_t* listelement_alloc(char* name, float x, float y, float width, float height, uint32_t color, char enablex, char enabley, char newatbottom, uint32_t buffersize, float scrollerwidth);

char listelement_reset(element_t* element);

char listelement_isempty(element_t* element);

char listelement_needstailitems(element_t* element);

char listelement_needsheaditems(element_t* element);

element_t* listelement_tailitem(element_t* element);

element_t* listelement_headitem(element_t* element);

char listelement_queuetailitem(element_t* element, element_t* item);

char listelement_queueheaditem(element_t* element, element_t* item);

void listelement_updateindexes(element_t* element, int headindex, int maxindex);

void listelement_autoscroll(element_t* element, input_t* input);

void listelement_update(element_t* element, input_t* input);

#endif

#if __INCLUDE_LEVEL__ == 0

/*

    A lag-free scrollable list.
    It sends out list.fillup commands.
    Its headerqueue and tailqueue must be filled up continuously.

*/

#include "zc_draw.c"
#include "zc_memory.c"
#include "zc_vector.c"

void listdata_dealloc(void* pointer);
void listelement_do_scroll(element_t* element, input_t* input, float y);
void listelement_input(element_t* element, input_t* input);

/* alloc list element */

element_t* listelement_alloc(
    char*    name,
    float    x,
    float    y,
    float    width,
    float    height,
    uint32_t color,
    char     enablex,
    char     enabley,
    char     newatbottom,
    uint32_t buffersize,
    float    scrollerwidth)
{
    bm_rgba_t* bitmap = bm_rgba_new(5, 5);
    gfx_rect(bitmap, 0, 0, 5, 5, color, 0);

    element_t*  element = element_alloc("list", name, x, y, width, height, bitmap);
    listdata_t* data    = CAL(sizeof(listdata_t), listdata_dealloc, NULL);

    data->list      = list_alloc(height, newatbottom == 1 ? kListAlignBottom : kListAlignTop);
    data->items     = VNEW();
    data->headqueue = VNEW();
    data->tailqueue = VNEW();
    data->scrollbar = solidelement_alloc("scrollbar", width - scrollerwidth, 0.0, scrollerwidth, height / 5.0, 0x000000FF);

    data->bounce      = 0;
    data->firepull    = 0;
    data->buffersize  = buffersize;
    data->newatbottom = newatbottom;

    data->dragging  = 0;
    data->scrolling = 0;
    data->rendering = 0;

    data->zoom      = 1.0;
    data->momentum  = 0.0;
    data->autospeed = 0.0;
    data->lasttouch = 0.0;

    data->translation       = v2_init(0.0, 0.0);
    data->scrolltranslation = v2_init(0.0, 0.0);

    data->scrollbar->translation = &data->scrolltranslation;

    element->data  = data;
    element->input = listelement_input;

    REL(bitmap);

    return element;
}

/* dealloc list */

void listdata_dealloc(void* pointer)
{
    listdata_t* list = pointer;

    REL(list->items);
    REL(list->headqueue);
    REL(list->tailqueue);
    REL(list->scrollbar);
}

/* resets list */

char listelement_reset(element_t* element)
{
    listdata_t* list = element->data;

    for (int index = 0; index < list->items->length; index++)
    {
	element_removesubelement(element, list->items->data[index]);
    }

    vec_reset(list->items);
    vec_reset(list->tailqueue);
    vec_reset(list->headqueue);

    list->bounce      = 0;
    list->momentum    = 0.0;
    list->translation = v2_init(0.0, 0.0);

    list_reset(list->list);

    return 1;
}

/* returns true if no items present */

char listelement_isempty(element_t* element)
{
    listdata_t* list = element->data;
    return list->items->length == 0;
}

/* returns true if head buffer is low */

char listelement_needsheaditems(element_t* element)
{
    listdata_t* list = element->data;
    return list->headqueue->length < list->buffersize;
}

/* returns true if tail buffer is low */

char listelement_needstailitems(element_t* element)
{
    listdata_t* list = element->data;
    return list->tailqueue->length < list->buffersize;
}

/* returns head item from buffer or from items if buffer is empty */

element_t* listelement_headitem(element_t* element)
{
    listdata_t* list = element->data;
    element_t*  head = NULL;
    if (list->headqueue->length > 0) head = vec_tail(list->headqueue);
    else head = vec_head(list->items);
    return head;
}

/* returns tail item from buffer or from items if buffer is empty */

element_t* listelement_tailitem(element_t* element)
{
    listdata_t* list = element->data;
    element_t*  tail = NULL;
    if (list->tailqueue->length > 0) tail = vec_tail(list->tailqueue);
    else tail = vec_tail(list->items);
    return tail;
}

/* adds one element to head buffer, returns 1 if buffer is full */

char listelement_queueheaditem(element_t* element, element_t* item)
{
    listdata_t* list = element->data;
    VADD(list->headqueue, item);
    if (list->headqueue->length == list->buffersize) return 1;
    else return 0;
}

/* adds one element to tail buffer, returns 1 if buffer is full */

char listelement_queuetailitem(element_t* element, element_t* item)
{
    listdata_t* list = element->data;
    VADD(list->tailqueue, item);
    if (list->tailqueue->length == list->buffersize) return 1;
    else return 0;
}

/* update indexes for scrollbar */

void listelement_updateindexes(element_t* element, int headindex, int maxindex)
{
    listdata_t* list = element->data;
    list_setheadindex(list->list, headindex, maxindex);
}

/* checks buffer sizes, sends out load requests if needed */

void listelement_autoscroll(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    list->autospeed = -10.0;
    cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);
}

/* checks buffer sizes, sends out load requests if needed */

void listelement_update(element_t* element, input_t* input)
{
    // listdata_t* list = element->data;

    listelement_do_scroll(element, input, 0.0);

    cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);
}

/* scroll elements */

void listelement_do_scroll(element_t* element, input_t* input, float y)
{
    listdata_t* list = element->data;

    char fillup = 0;

    if (y > 100.0) y = 100.0;
    if (y < -100.0) y = -100.0;

    list->translation.y += y;
    list->rendering = 0;

    list_scroll(list->list, y);

    while (list->list->needshead == 1)
    {
	if (list->headqueue->length < list->buffersize) fillup = 1;
	if (list->headqueue->length > 0)
	{
	    element_t* newhead = list->headqueue->data[0];
	    if (newhead->texture.ready == 1 || newhead->texture.tiled == 0)
	    {
		vec_ins(list->items, newhead, 0);
		vec_rem_at_index(list->headqueue, 0);
		element_addsubelementatindex(element, newhead, 0);

		list_addhead(list->list, newhead->height);
		element_setposition(newhead, 0, list->list->items[0].position);
		element_settranslation(newhead, &list->translation);

		input->upload |= 1;
	    }
	    else
	    {
		list->rendering = 1;
		break;
	    }
	}
	else break;
    }

    while (list->list->removehead == 1 && list->items > 0)
    {
	element_t* head = vec_head(list->items);

	vec_ins(list->headqueue, head, 0);
	vec_rem_at_index(list->items, 0);
	element_removesubelement(element, head);

	list_removehead(list->list, head->height);
	if (list->headqueue->length > list->buffersize) vec_rem_at_index(list->headqueue, list->headqueue->length - 1);

	input->upload = 1;
    }

    while (list->list->needstail == 1)
    {
	if (list->tailqueue->length < list->buffersize) fillup = 1;
	if (list->tailqueue->length > 0)
	{
	    element_t* newtail = list->tailqueue->data[0];
	    if (newtail->texture.ready == 1 || newtail->texture.tiled == 0)
	    {
		VADD(list->items, newtail);
		vec_rem_at_index(list->tailqueue, 0);
		element_addsubelementatindex(element, newtail, list->items->length - 1);

		list_addtail(list->list, newtail->height);
		element_setposition(newtail, 0, list->list->items[list->list->length - 1].position);
		element_settranslation(newtail, &list->translation);

		input->upload |= 1;
	    }
	    else
	    {
		list->rendering = 1;
		break;
	    }
	}
	else break;
    }

    while (list->list->removetail == 1 && list->items > 0)
    {
	element_t* tail = vec_tail(list->items);

	vec_ins(list->tailqueue, tail, 0);
	vec_rem_at_index(list->items, list->items->length - 1);
	element_removesubelement(element, tail);

	list_removetail(list->list, tail->height);
	if (list->tailqueue->length > list->buffersize) vec_rem_at_index(list->tailqueue, list->tailqueue->length - 1);

	input->upload = 1;
    }

    if (list->items->length > 0)
    {
	element_setposition(list->scrollbar, element->width - list->scrollbar->width, list->scrollbar->y);
	list->scrolltranslation.y = (element->height - list->scrollbar->height) * list->list->scrollratio;
    }

    if (fillup == 1) cmdqueue_addonce(input->cmdqueue, "list.fillup", element, NULL);

    input->render = 1;
}

/* zoom elements */

void listelement_do_zoom(element_t* element, float delta, input_t* input)
{
    listdata_t* data = element->data;

    if (data->items->length > 0)
    {
	list_zoom(data->list, delta);

	for (int index = 0; index < data->list->length; index++)
	{
	    listitem_t listitem = data->list->items[index];
	    element_t* realitem = data->items->data[index];
	    realitem->y         = listitem.position;
	}

	input->upload = 1;

	listelement_do_scroll(element, input, 0.0);
    }
}

/* touch down event, storing coordinates */

void listelement_touch_down(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    list->lasttouch = input->floatb;

    if (list->list->itemsize > list->list->listsize)
    {
	element_addsubelement(element, list->scrollbar);
	list->scrollbar->translation = &list->scrolltranslation;
    }

    // enable re-request

    list->dragging = 1;

    input->upload = 1;
}

/* touch up event, storing coordinates */

void listelement_touch_up(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    list->dragging = 0;

    if (list->firepull == 1)
    {
	list->firepull   = 0;
	char* onpulldown = MGET(element->actions, "onpulldown");
	if (onpulldown != NULL) cmdqueue_add(input->cmdqueue, onpulldown, element, NULL);
    }

    cmdqueue_add(input->cmdqueue, "list.actionend", element, NULL);
    cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);

    if (list->list->zoom == 0.0)
    {
	char* onzoomdown = MGET(element->actions, "onzoomdown");
	if (onzoomdown != NULL) cmdqueue_add(input->cmdqueue, onzoomdown, element, NULL);
    }
    else if (list->list->zoom > 1.0)
    {
	char* onzoomup = MGET(element->actions, "onzoomup");
	if (onzoomup != NULL) cmdqueue_add(input->cmdqueue, onzoomup, element, NULL);
    }

    if (list->list->zoom == 0.0 || list->list->zoom > 1.0)
    {
	list_setzoom(list->list, 1.0);

	for (int index = 0; index < list->list->length; index++)
	{
	    listitem_t listitem = list->list->items[index];
	    element_t* realitem = list->items->data[index];
	    realitem->y         = listitem.position;
	}

	input->upload = 1;
	listelement_do_scroll(element, input, 0.0);
    }
}

/* zoom event */

void listelement_zoomend(element_t* element, input_t* input)
{
    listdata_t* list = element->data;
    if (list->list->zoom == 0.0)
    {
	char* onzoomdown = MGET(element->actions, "onzoomdown");
	if (onzoomdown != NULL) cmdqueue_add(input->cmdqueue, onzoomdown, element, NULL);
    }
    else if (list->list->zoom > 1.0)
    {
	char* onzoomup = MGET(element->actions, "onzoomup");
	if (onzoomup != NULL) cmdqueue_add(input->cmdqueue, onzoomup, element, NULL);
    }

    if (list->list->zoom == 0.0 || list->list->zoom > 1.0)
    {
	list_setzoom(list->list, 1.0);

	for (int index = 0; index < list->list->length; index++)
	{
	    listitem_t listitem = list->list->items[index];
	    element_t* realitem = list->items->data[index];
	    realitem->y         = listitem.position;
	}

	input->upload = 1;
	listelement_do_scroll(element, input, 0.0);
    }
}

/* touch drag event */

void listelement_touch_drag(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    list->momentum  = input->floatb - list->lasttouch;
    list->lasttouch = input->floatb;

    listelement_do_scroll(element, input, list->momentum);
    if (list->translation.y > 100.0) list->firepull = 1;
    input->render = 1;
}

/* scroll start event */

void listelement_scroll_start(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    if (list->list->itemsize > list->list->listsize)
    {
	element_addsubelement(element, list->scrollbar);
	list->scrollbar->translation = &list->scrolltranslation;
    }

    list->bounce    = 0;
    list->scrolling = 1;

    input->upload = 1;
}

/* scroll end event */

void listelement_scroll_end(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    list->scrolling = 0;
    list->momentum  = 0.0;

    if (list->items->length > 0)
    {
	element_t* first    = list->items->data[0];
	float      position = first->y;

	if (list->zoom < 0.75) list->zoom = 0.5;
	else list->zoom = 1.0;

	for (int index = 0; index < list->items->length; index++)
	{
	    element_t* item = list->items->data[index];
	    item->y         = position;

	    position += item->height * list->zoom;
	}

	listelement_do_scroll(element, input, 0.0);
    }

    list->bounce = 0;

    cmdqueue_add(input->cmdqueue, "list.actionend", element, NULL);
    cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);

    if (list->firepull == 1)
    {
	list->firepull   = 0;
	char* onpulldown = MGET(element->actions, "onpulldown");

	if (onpulldown != NULL) cmdqueue_add(input->cmdqueue, onpulldown, element, NULL);
    }
}

/* scroll event */

void listelement_scroll(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    if (input->chara == 0)
    {
	if ((list->list->needshead == 1 && input->floatb > 0) ||
	    (list->list->needstail == 1 && input->floatb < 0)) list->bounce = 1;

	list->momentum = input->floatb;
	if (list->bounce == 1)
	{
	    if (list->momentum > 10.0) list->momentum = 10.0;
	    if (list->momentum < -10.0) list->momentum = -10.0;
	}
	listelement_do_scroll(element, input, list->momentum);
	if (list->translation.y > 100.0) list->firepull = 1;
    }
    else
    {
	// listelement_do_zoom( element , input->floata  , input->floatb , input );
    }
}

/* zoom event */

void listelement_zoom(element_t* element, input_t* input)
{
    listelement_do_zoom(element, input->floatb, input);
}

/* timer event, returns finished state */

void listelement_timer(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    if (list->dragging == 1 || list->scrolling == 1) return;

    /* slow down scrolling */

    list->momentum *= .95;

    listelement_do_scroll(element, input, list->momentum + list->autospeed);

    /* bounce at edges */

    if (list->items->length > 0 && list->rendering == 0)
    {
	if (list->list->needshead == 1 && list->list->needstail == 1)
	{
	    if (list->list->align == kListAlignBottom)
	    {
		element_t* tail = vec_tail(list->items);
		list->momentum  = (element->height - (list->translation.y + tail->y + tail->height)) / 10.0;
		list->bounce    = 1;
		list->autospeed = 0.0;
	    }
	    else
	    {
		element_t* head = vec_head(list->items);
		list->momentum  = -(list->translation.y + head->y) / 10.0;
		list->bounce    = 1;
		list->autospeed = 0.0;
	    }
	}
	else if (list->list->needshead == 1)
	{
	    element_t* head = vec_head(list->items);
	    list->momentum  = -(list->translation.y + head->y) / 10.0;
	    list->bounce    = 1;
	    list->autospeed = 0.0;
	}
	else if (list->list->needstail == 1)
	{
	    element_t* head = vec_head(list->items);
	    element_t* tail = vec_tail(list->items);
	    list->momentum  = (element->height - (list->translation.y + tail->y + tail->height)) / 10.0;
	    if (list->list->align == kListAlignTop) list->momentum = -(list->translation.y + head->y) / 10.0;
	    list->bounce    = 1;
	    list->autospeed = 0.0;
	}
    }

    /* check for finished */

    if (list->momentum < .1 && list->momentum > -.1 && list->autospeed == 0.0)
    {
	list->momentum = 0.0;
	if (list->items->length > 0)
	{
	    element_t* head = vec_head(list->items);
	    element_t* tail = vec_tail(list->items);

	    if (list->translation.y + head->y < 5.0 || list->translation.y + tail->y + tail->height > element->height - 5.0)
	    {
		element_removesubelement(element, list->scrollbar);
		input->upload |= 1;
		list->bounce = 0;
	    }
	}

	if (list->rendering == 0) cmdqueue_add(input->cmdqueue, "ui.removeastimed", element, NULL);
    }
}

/* resize listelement */

void listelement_resize(element_t* element, input_t* input)
{
    listdata_t* list = element->data;

    element_input(element, input);
    list_setheight(list->list, element->height);

    for (int index = 0; index < list->items->length; index++)
    {
	element_t* item = list->items->data[index];
	item->input(item, input);
    }

    for (int index = 0; index < list->headqueue->length; index++)
    {
	element_t* item = list->headqueue->data[index];
	item->input(item, input);
    }

    for (int index = 0; index < list->tailqueue->length; index++)
    {
	element_t* item = list->tailqueue->data[index];
	item->input(item, input);
    }
}

/* input event */

void listelement_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeTimer: listelement_timer(element, input); break;
	case kInputTypeTouchDown: listelement_touch_down(element, input); break;
	case kInputTypeTouchUp: listelement_touch_up(element, input); break;
	case kInputTypeTouchDrag: listelement_touch_drag(element, input); break;
	case kInputTypeScrollStart: listelement_scroll_start(element, input); break;
	case kInputTypeScroll: listelement_scroll(element, input); break;
	case kInputTypeScrollEnd: listelement_scroll_end(element, input); break;
	case kInputTypeZoom: listelement_zoom(element, input); break;
	case kInputTypeZoomEnd: listelement_zoomend(element, input); break;
	case kInputTypeResize: listelement_resize(element, input); break;
	default: element_input(element, input); break;
    }
}

#endif
