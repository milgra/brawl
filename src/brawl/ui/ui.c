#include "ui.h"
#include "zc_cstring.c"
#include "zc_map.c"
#include "zc_memory.c"
#include <assert.h>

void uitouch_dealloc(void* pointer);
void ui_dealloc(void* pointer);

/* create uitouch */

uitouch_t* uitouch_alloc(char* id, v2_t start)
{
    uitouch_t* touch = CAL(sizeof(uitouch_t), uitouch_dealloc, NULL);

    touch->id    = cstr_new_cstring(id);
    touch->views = VNEW();
    touch->start = touch->actual = touch->last = start;

    return touch;
}

/* delete uitouch */

void uitouch_dealloc(void* pointer)
{
    uitouch_t* touch = pointer;

    REL(touch->id);
    REL(touch->views);
}

/* create ui */

ui_t* ui_alloc(float swipedist)
{
    ui_t* ui = CAL(sizeof(ui_t), ui_dealloc, NULL);

    ui->touches  = MNEW();
    ui->timed    = VNEW();
    ui->visible  = VNEW();
    ui->focused  = VNEW();
    ui->scrolled = VNEW();
    ui->zoomed   = VNEW();

    ui->swipedist = swipedist;

    return ui;
}

/* delete ui */

void ui_dealloc(void* pointer)
{
    ui_t* ui = pointer;

    REL(ui->touches);
    REL(ui->timed);
    REL(ui->visible);
    REL(ui->focused);
    REL(ui->scrolled);
    REL(ui->zoomed);
}

/* sets focus */

void ui_setfocused(ui_t* ui, element_t* element, input_t* input)
{
    /* remove focus from previously focused views */

    for (int index = 0; index < ui->focused->length; index++)
    {
	element_t* element    = ui->focused->data[index];
	input_t    focusinput = *input;
	focusinput.type       = kInputTypeUnFocus;
	element->input(element, &focusinput);
    }

    vec_reset(ui->focused);

    if (element != NULL)
    {
	input_t focusinput = *input;
	focusinput.type    = kInputTypeFocus;
	element->input(element, &focusinput);
	vec_add_unique_data(ui->focused, element);
    }
}

/* add timed view */

void ui_addtimed(ui_t* ui, element_t* element)
{
    vec_add_unique_data(ui->timed, element);
}

/* remove timed view */

void ui_removetimed(ui_t* ui, element_t* element)
{
    VREM(ui->timed, element);
}

/* touch down event */

void ui_touch_down(ui_t* ui, input_t* input)
{
    float x = input->floata;
    float y = input->floatb;

    assert(MGET(ui->touches, input->stringa) == NULL);

    uitouch_t* touch = uitouch_alloc(input->stringa, v2_init(x, y));
    MPUT(ui->touches, input->stringa, touch);
    REL(touch);

    for (int index = 0; index < ui->visible->length; index++)
    {
	element_t* element = ui->visible->data[index];

	v2_t trans = v2_init(0.0, 0.0);
	if (element->translation != NULL) trans = *element->translation;

	if (x >= (trans.x + element->finalx) &&
	    x <= (trans.x + element->finalx + element->width) &&
	    y >= (trans.y + element->finaly) &&
	    y <= (trans.y + element->finaly + element->height))
	{
	    if (element->notouchunder == 1) vec_reset(touch->views);
	    VADD(touch->views, element);
	}
    }

    /* input touch down event */

    for (int index = 0; index < touch->views->length; index++)
    {
	element_t* element = touch->views->data[index];
	element->input(element, input);
    }

    /* input touch outside event for focused */

    for (int index = 0; index < ui->focused->length; index++)
    {
	element_t* element = ui->focused->data[index];
	v2_t       trans   = v2_init(0.0, 0.0);
	if (element->translation != NULL) trans = *element->translation;

	if (x >= (trans.x + element->finalx) &&
	    x <= (trans.x + element->finalx + element->width) &&
	    y >= (trans.y + element->finaly) &&
	    y <= (trans.y + element->finaly + element->height))
	{
	}
	else
	{
	    input_t touchinput = *input;
	    touchinput.type    = kInputTypeTouchDownOutside;
	    element->input(element, &touchinput);
	}
    }
}

/* touch drag event */

void ui_touch_drag(ui_t* ui, input_t* input)
{
    if (ui->touches->count == 1)
    {
	uitouch_t* touch = MGET(ui->touches, input->stringa);

	if (touch != NULL)
	{
	    touch->last = v2_init(input->floata, input->floatb);

	    /* check for swipe */

	    if (touch->swiped == 0)
	    {
		if (fabs(input->floata - touch->start.x) > ui->swipedist)
		{
		    touch->swiped      = 1;
		    input_t swipeinput = *input;
		    swipeinput.type    = input->floata - touch->start.x > 0 ? kInputTypeRightSwipe : kInputTypeLeftSwipe;
		    for (int index = 0; index < touch->views->length; index++)
		    {
			element_t* element = touch->views->data[index];
			element->input(element, &swipeinput);
		    }
		}
	    }

	    /* normal drag input */

	    for (int index = 0; index < touch->views->length; index++)
	    {
		element_t* element = touch->views->data[index];
		element->input(element, input);
	    }
	}
    }
    else if (ui->touches->count == 2)
    {
	vec_t* keys = VNEW();
	map_keys(ui->touches, keys);

	uitouch_t* touch = MGET(ui->touches, input->stringa);

	touch->last = v2_init(input->floata, input->floatb);

	uitouch_t* toucha = MGET(ui->touches, keys->data[0]);
	uitouch_t* touchb = MGET(ui->touches, keys->data[1]);

	REL(keys);

	float oldx = fabs(touchb->actual.x - toucha->actual.x);
	float oldy = fabs(touchb->actual.y - toucha->actual.y);

	float actualx = fabs(touchb->last.x - toucha->last.x);
	float actualy = fabs(touchb->last.y - toucha->last.y);

	input_t zoominput = *input;
	zoominput.type    = kInputTypeZoom;
	zoominput.floata  = (actualx - oldx) / 1000.0;
	zoominput.floatb  = (actualy - oldy) / 1000.0;

	for (int index = 0; index < toucha->views->length; index++)
	{
	    element_t* element = toucha->views->data[index];
	    element->input(element, &zoominput);
	}

	for (int index = 0; index < touchb->views->length; index++)
	{
	    element_t* element = touchb->views->data[index];
	    element->input(element, &zoominput);
	}

	input->upload |= zoominput.upload;
	input->render |= zoominput.render;

	toucha->actual = toucha->last;
	touchb->actual = touchb->last;
    }
}

/* touch up event */

void ui_touch_up(ui_t* ui, input_t* input)
{
    uitouch_t* touch = MGET(ui->touches, input->stringa);

    if (touch != NULL)
    {
	for (int index = 0; index < touch->views->length; index++)
	{
	    element_t* element = touch->views->data[index];
	    element->input(element, input);
	}
    }

    MDEL(ui->touches, input->stringa);
}

/* zoom start event */

void ui_zoom_start(ui_t* ui, input_t* input)
{
    float x = input->floata;
    float y = input->floatb;

    /* collect scrolled views */

    vec_reset(ui->zoomed);

    for (int index = 0; index < ui->visible->length; index++)
    {
	element_t* element = ui->visible->data[index];
	v2_t       trans   = v2_init(0.0, 0.0);
	if (element->translation != NULL) trans = *element->translation;
	if (x >= (trans.x + element->finalx) &&
	    x <= (trans.x + element->finalx + element->width) &&
	    y >= (trans.y + element->finaly) &&
	    y <= (trans.y + element->finaly + element->height))
	{
	    VADD(ui->zoomed, element);
	}
    }

    for (int index = 0; index < ui->zoomed->length; index++)
    {
	element_t* element = ui->zoomed->data[index];
	element->input(element, input);
    }
}

/* zoom event */

void ui_zoom(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->zoomed->length; index++)
    {
	element_t* element = ui->zoomed->data[index];
	element->input(element, input);
    }
}

/* zoom end event */

void ui_zoom_end(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->zoomed->length; index++)
    {
	element_t* element = ui->zoomed->data[index];
	element->input(element, input);
    }
}

/* scroll start event */

void ui_scroll_start(ui_t* ui, input_t* input)
{
    float x = input->floata;
    float y = input->floatb;

    /* collect scrolled views */

    vec_reset(ui->scrolled);

    for (int index = 0; index < ui->visible->length; index++)
    {
	element_t* element = ui->visible->data[index];
	v2_t       trans   = v2_init(0.0, 0.0);
	if (element->translation != NULL) trans = *element->translation;
	if (x >= (trans.x + element->finalx) &&
	    x <= (trans.x + element->finalx + element->width) &&
	    y >= (trans.y + element->finaly) &&
	    y <= (trans.y + element->finaly + element->height))
	{
	    VADD(ui->scrolled, element);
	}
    }

    for (int index = 0; index < ui->scrolled->length; index++)
    {
	element_t* element = ui->scrolled->data[index];
	element->input(element, input);
    }
}

/* scroll event */

void ui_scroll(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->scrolled->length; index++)
    {
	element_t* element = ui->scrolled->data[index];
	element->input(element, input);
    }
}

/* scroll end event */

void ui_scroll_end(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->scrolled->length; index++)
    {
	element_t* element = ui->scrolled->data[index];
	element->input(element, input);
    }
}

/* keypress event */

void ui_keypress(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->focused->length; index++)
    {
	element_t* element = ui->focused->data[index];
	element->input(element, input);
    }
}

/* timer event */

void ui_timer(ui_t* ui, input_t* input)
{
    for (int index = 0; index < ui->timed->length; index++)
    {
	element_t* element = ui->timed->data[index];
	element->input(element, input);
    }
}
