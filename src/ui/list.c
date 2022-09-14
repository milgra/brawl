

#include "list.h"
#include "zc_memory.c"

void list_dealloc(void* pointer);

/* alloc list */

list_t* list_alloc(float height, char align)
{
    list_t* list = CAL(sizeof(list_t), list_dealloc, NULL);

    list->length      = 0;
    list->length_real = 10;
    list->items       = CAL(sizeof(listitem_t) * list->length_real, NULL, NULL);
    list->listsize    = height;
    list->align       = align;
    list->zoom        = 1.0;
    list->minrowsize  = 50.0;

    return list;
}

/* dealloc list */

void list_dealloc(void* pointer)
{
    list_t* list = pointer;

    REL(list->items);
}

/* resets list */

void list_reset(list_t* list)
{
    list->length      = 0;
    list->itemsize    = 0;
    list->maxindex    = 0;
    list->headindex   = 0;
    list->translation = 0;
}

/* set list height */

void list_setheight(list_t* list, float height)
{
    list->listsize = height;
}

/* sets index of head item */

void list_setheadindex(list_t* list, uint32_t index, uint32_t max)
{
    list->maxindex  = max;
    list->headindex = index;
}

/* internal - extend listitem container */

void list_extend(list_t* list)
{
    list->length_real += 10;
    list->items = mem_realloc(list->items, sizeof(listitem_t) * list->length_real);
}

/* add head item */

void list_addhead(list_t* list, float height)
{
    list->length += 1;
    list->headindex -= 1;

    if (list->length == list->length_real) list_extend(list);

    for (int index = list->length - 1; index > 0; index--) list->items[index] = list->items[index - 1];

    list->items[0].position     = 0.0;
    list->items[0].height       = height;
    list->items[0].zoomedheight = list->minrowsize + (height - list->minrowsize) * list->zoom;

    if (list->length > 1)
    {
	list->items[0].position = list->items[1].position - list->items[0].zoomedheight;
    }
    else if (list->align == kListAlignBottom)
    {
	list->items[0].position = list->listsize - list->items[0].zoomedheight;
    }

    /* calculate itemsize */

    list->itemsize = 0;
    for (int index = 0; index < list->length; index++)
    {
	listitem_t* item = &list->items[index];
	list->itemsize += item->zoomedheight;
    }

    list_scroll(list, 0.0);
}

/* add tail item */

void list_addtail(list_t* list, float height)
{
    list->length += 1;

    if (list->length == list->length_real) list_extend(list);

    list->items[list->length - 1].position     = 0.0;
    list->items[list->length - 1].height       = height;
    list->items[list->length - 1].zoomedheight = list->minrowsize + (height - list->minrowsize) * list->zoom;

    if (list->length > 1)
    {
	list->items[list->length - 1].position = list->items[list->length - 2].position + list->items[list->length - 2].zoomedheight;
    }

    /* calculate itemsize */

    list->itemsize = 0;
    for (int index = 0; index < list->length; index++)
    {
	listitem_t* item = &list->items[index];
	list->itemsize += item->zoomedheight;
    }

    list_scroll(list, 0.0);
}

/* remove head item */

void list_removehead(list_t* list, float height)
{
    if (list->length > 0)
    {
	list->headindex += 1;
	list->length -= 1;

	for (int index = 0; index < list->length; index++) list->items[index] = list->items[index + 1];

	list->itemsize = 0;
	for (int index = 0; index < list->length; index++)
	{
	    listitem_t* item = &list->items[index];
	    list->itemsize += item->zoomedheight;
	}

	list_scroll(list, 0.0);
    }
}

/* remove tail item */

void list_removetail(list_t* list, float height)
{
    if (list->length > 0)
    {
	list->length -= 1;

	/* calculate itemsize */

	list->itemsize = 0;
	for (int index = 0; index < list->length; index++)
	{
	    listitem_t* item = &list->items[index];
	    list->itemsize += item->zoomedheight;
	}

	list_scroll(list, 0.0);
    }
}

/* set zoom */

void list_setzoom(list_t* list, float value)
{
    list->zoom = value;

    if (list->zoom < 0.0) list->zoom = 0.0;
    if (list->zoom > 1.5) list->zoom = 1.5;

    listitem_t* head     = &list->items[0];
    float       position = head->position;

    for (int index = 0; index < list->length; index++)
    {
	listitem_t* item   = &list->items[index];
	item->position     = position;
	item->zoomedheight = list->minrowsize + (item->height - list->minrowsize) * list->zoom;

	position += item->zoomedheight;
    }

    list->itemsize = 0;
    for (int index = 0; index < list->length; index++)
    {
	listitem_t* item = &list->items[index];
	list->itemsize += item->zoomedheight;
    }
}

/* zoom items */

void list_zoom(list_t* list, float delta)
{
    list->zoom += delta;

    list_setzoom(list, list->zoom);
}

/* scroll, set items to add/remove, update scroll ratio */

void list_scroll(list_t* list, float delta)
{
    list->translation += delta;

    list->needshead  = 0;
    list->needstail  = 0;
    list->removehead = 0;
    list->removetail = 0;

    if (list->length > 0)
    {
	listitem_t* head = &list->items[0];
	listitem_t* tail = &list->items[list->length - 1];

	float headtop = list->translation + head->position;
	float headend = list->translation + head->position + head->zoomedheight;
	float tailtop = list->translation + tail->position;
	float tailend = list->translation + tail->position + tail->zoomedheight;

	if (headtop > 0) list->needshead = 1;
	else if (headend < 0 && list->itemsize > list->listsize) list->removehead = 1;

	if (tailend < list->listsize) list->needstail = 1;
	else if (tailtop > list->listsize && list->itemsize > list->listsize) list->removetail = 1;

	float headindex = list->headindex;
	float tailindex = list->headindex + list->length;

	headindex += 1.0 - headend / (head->zoomedheight);
	tailindex -= (tailend - list->listsize) / (tail->zoomedheight);

	float visible = tailindex - headindex;

	list->scrollratio = headindex / (list->maxindex - visible);
    }
    else
    {
	list->needshead = 1;
	list->needstail = 1;
    }
}
