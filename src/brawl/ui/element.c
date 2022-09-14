

#include "element.h"
#include "floatbuffer.c"
#include "image.c"
#include "zc_cstring.c"
#include "zc_mat4.c"
#include "zc_memory.c"
#include "zc_string.c"

void element_dealloc(void* pointer);

/* alloc element */

element_t* element_alloc(char* type, char* name, float x, float y, float width, float height, mtbmp_t* bitmap)
{
    element_t* element = CAL(sizeof(element_t), element_dealloc, NULL);

    element->focused      = 0;
    element->exclude      = 0;
    element->notouchunder = 0;

    element->type = cstr_new_cstring(type);
    element->name = cstr_new_cstring(name);
    element->data = NULL;

    element->x = x;
    element->y = y;

    element->width  = width;
    element->height = height;

    element->finalx = x;
    element->finaly = y;

    if (bitmap) element->bitmap = RET(bitmap);
    element->actions     = MNEW();
    element->subelements = VNEW();
    element->translation = NULL;

    element->texture.tiled             = 1;
    element->texture.name              = 0;
    element->texture.ready             = 1;
    element->texture.sampler           = 0;
    element->texture.coordstamp.tv_sec = 0;

    element->input = element_input;

    return element;
}

/* dealloc element */

void element_dealloc(void* pointer)
{
    element_t* element = pointer;

    REL(element->type);
    REL(element->name);

    if (element->data)
	REL(element->data);
    if (element->bitmap)
	REL(element->bitmap);
    REL(element->actions);

    for (int index = 0; index < element->subelements->length; index++) element_removesubelement(element, element->subelements->data[index]);

    REL(element->subelements);
}

/* set type */

void element_settype(element_t* element, char* type)
{
    REL(element->type);
    element->type = cstr_new_cstring(type);
}

/* set data */

void element_setdata(element_t* element, void* data)
{
    if (element->data) REL(element->data);
    if (data) element->data = RET(data);
}

/* set bitmap */

void element_setbitmap(element_t* element, mtbmp_t* bitmap)
{
    if (element->bitmap) REL(element->bitmap);
    element->bitmap                    = RET(bitmap);
    element->texture.coordstamp.tv_sec = 0;
}

/* sets an action for a specific action id */

void element_setaction(element_t* element, char* actionid, char* action)
{
    char* actioncpy = cstr_new_cstring(action);
    MPUT(element->actions, actionid, actioncpy);
    REL(actioncpy);
}

/* set focused state */

void element_setfocused(element_t* element, char flag)
{
    element->focused = flag;
}

/* sets position */

void element_setposition(element_t* element, float x, float y)
{
    element->x = x;
    element->y = y;
}

/* sets dimension */

void element_setdimension(element_t* element, float width, float height)
{
    element->width  = width;
    element->height = height;
}

/* sets translation recursively */

void element_settranslation(element_t* element, v2_t* translation)
{
    element->translation = translation;
    for (int index = 0; index < element->subelements->length; index++) element_settranslation(element->subelements->data[index], translation);
}

/* add subelement */

void element_addsubelement(element_t* element, element_t* subelement)
{
    vec_add_unique_data(element->subelements, subelement);
    subelement->translation = element->translation;
}

/* add subelement at inde */

void element_addsubelementatindex(element_t* element, element_t* subelement, size_t index)
{
    vec_ins_unique_data(element->subelements, subelement, index);
    subelement->translation = element->translation;
}

/* remove subelement */

void element_removesubelement(element_t* element, element_t* subelement)
{
    VREM(element->subelements, subelement);
}

/* collect all elements recursively */

void element_collectelements(element_t* element, v2_t parent, vec_t* vector)
{
    element->finalx = element->x + parent.x;
    element->finaly = element->y + parent.y;

    parent.x = element->finalx;
    parent.y = element->finaly;

    if (element->exclude == 0) VADD(vector, element);
    for (int index = 0; index < element->subelements->length; index++) element_collectelements(element->subelements->data[index], parent, vector);
}

/* animate element to position */

void element_animateto(element_t* element, float x, float y, int delay, char* action)
{
    element->animation.target.x = x - element->x;
    element->animation.target.y = y - element->y;

    element->animation.trans.x = 0.0;
    element->animation.trans.y = 0.0;

    element->animation.delay  = delay;
    element->animation.action = action;

    if (element->translation == NULL)
    {
	element->translation               = &element->animation.trans;
	element->texture.name              = 0;
	element->texture.coordstamp.tv_sec = 0;
    }
}

/* timer event, animate if needed */

void element_timer(element_t* element, input_t* input)
{
    if (element->translation != NULL)
    {
	if (element->animation.delay > 0) element->animation.delay--;
	else
	{
	    element->animation.trans.x += (element->animation.target.x - element->animation.trans.x) / 3.0;
	    element->animation.trans.y += (element->animation.target.y - element->animation.trans.y) / 3.0;

	    if (fabs(element->animation.target.x - element->animation.trans.x) < .1 && fabs(element->animation.target.y - element->animation.trans.y) < .1)
	    {
		if (element->animation.action != NULL) cmdqueue_add(input->cmdqueue, element->animation.action, NULL, NULL);
		element->x += element->animation.target.x;
		element->y += element->animation.target.y;
		element->animation.action = NULL;

		element->translation               = NULL;
		element->texture.name              = 0;
		element->texture.coordstamp.tv_sec = 0;

		input->upload = 1;

		cmdqueue_add(input->cmdqueue, "ui.removeastimed", element, NULL);
	    }
	}
	input->render = 1;
    }
}

/* input event */

void element_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeTimer:
	{
	    element_timer(element, input);
	    break;
	}
	case kInputTypeResize:
	{
	    if (element->autosize.fillx > 0.0) element->width = input->floata * element->autosize.fillx;
	    if (element->autosize.filly > 0.0) element->height = input->floatb * element->autosize.filly;

	    if (element->autosize.keepxcenter == 1) element->x = input->floata / 2.0 - element->width / 2.0;
	    if (element->autosize.keepycenter == 1) element->y = input->floatb / 2.0 - element->height / 2.0;

	    if (element->autosize.topmargin > 0.0) element->y = element->autosize.topmargin;
	    if (element->autosize.bottommargin > 0.0) element->y = input->floatb - element->height - element->autosize.bottommargin;
	    if (element->autosize.leftmargin > 0.0) element->x = element->autosize.leftmargin;
	    if (element->autosize.rightmargin > 0.0) element->x = input->floata - element->width - element->autosize.rightmargin;

	    if (element->autosize.fillx == 1 || element->autosize.filly == 1 || element->autosize.keepxcenter == 1 || element->autosize.keepycenter == 1)
	    {
		for (int index = 0; index < element->subelements->length; index++)
		{
		    element_t* subelement = element->subelements->data[index];
		    subelement->input(subelement, input);
		}
	    }
	    break;
	}
	case kInputTypeTouchDown:
	{
	    char* ontouchdown = MGET(element->actions, "ontouchdown");
	    if (ontouchdown != NULL) cmdqueue_add(input->cmdqueue, ontouchdown, element, NULL);
	    break;
	}
	case kInputTypeTouchUp:
	{
	    char* ontouchup = MGET(element->actions, "ontouchup");
	    if (ontouchup != NULL) cmdqueue_add(input->cmdqueue, ontouchup, element, NULL);
	    break;
	}
	case kInputTypeRightSwipe:
	{
	    char* onswiperight = MGET(element->actions, "onswiperight");
	    if (onswiperight != NULL) cmdqueue_add(input->cmdqueue, onswiperight, element, NULL);
	    break;
	}
	case kInputTypeLeftSwipe:
	{
	    char* onswipeleft = MGET(element->actions, "onswipeleft");
	    if (onswipeleft != NULL) cmdqueue_add(input->cmdqueue, onswipeleft, element, NULL);
	    break;
	}
    }
}

/* allocs solidcolor element */

element_t* solidelement_alloc(
    char*    name,
    float    x,
    float    y,
    float    width,
    float    height,
    uint32_t color)
{
    mtbmp_t* bitmap = mtbmp_alloc(ceilf(width) + 1, ceilf(height) + 1);
    mtbmp_fill_with_color(bitmap, 0, 0, bitmap->width, bitmap->height, color);

    element_t* element = element_alloc("solid", name, x, y, width, height, bitmap);

    REL(bitmap);

    return element;
}

/* sets color of solidelement */

void solidelement_setcolor(element_t* element, uint32_t color)
{
    mtbmp_t* bitmap = mtbmp_alloc(ceilf(element->width), ceilf(element->height));
    mtbmp_fill_with_color(bitmap, 0, 0, bitmap->width, bitmap->height, color);
    element_setbitmap(element, bitmap);
    REL(bitmap);
}

/* allocs image element */

element_t* imageelement_alloc(
    char* name,
    float x,
    float y,
    float width,
    float height,
    char* imagepath)
{
    mtbmp_t*   bitmap  = image_bmp_from_png(imagepath);
    element_t* element = element_alloc("image", name, x, y, width, height, bitmap);
    REL(bitmap);

    return element;
}

/* resizes image element */

void imageelement_resize(element_t* element, float width, float height)
{
    float imgratio = element->bitmap->width / element->bitmap->height;
    float newratio = width / height;

    if (imgratio > newratio)
    {
	/* fit into wanted width */
	element->width  = width;
	element->height = width * (element->bitmap->height / element->bitmap->width);
    }
    else
    {
	/* fit into wanted height */
	element->width  = height * imgratio;
	element->height = height;
	element->x      = (width - element->width) / 2.0;
    }
}

void videoelement_input(element_t* element, input_t* input);

/* allocs video element */

element_t* videoelement_alloc(
    char*  name,
    float  x,
    float  y,
    float  width,
    float  height,
    str_t* path)
{
    element_t* element = element_alloc("video", name, x, y, width, height, NULL);

    if (path)
	element->data = RET(path);
    element->input = videoelement_input;

    element->texture.tiled             = 0;
    element->texture.coordstamp.tv_sec = 0;

    element_setdata(element, path);

    return element;
}

/* resizes video element */

void videoelement_resize(element_t* element, float width, float height)
{
    float realwth  = 320;
    float realhth  = 200;
    float imgratio = realwth / realhth;
    float newratio = width / height;

    if (imgratio > newratio)
    {
	/* fit into wanted width */
	element->width  = width;
	element->height = width * (realhth / realwth);
    }
    else
    {
	/* fit into wanted height */
	element->width  = height * imgratio;
	element->height = height;
	element->x      = (width - element->width) / 2.0;
    }
}

/* input event, always draw in case of timer */

void videoelement_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeTimer:
	    input->render = 1;
	    break;
	default:
	    element_input(element, input);
	    break;
    }
}
