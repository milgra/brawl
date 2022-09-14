#ifndef notifierelement_h
#define notifierelement_h

#include "element.c"
#include "vectoranimator.c"
#include <stdio.h>

#define kNotifierStateAppear 0
#define kNotifierStateIdle 1
#define kNotifierStateDisappear 2

typedef struct _notifierdata_t
{
    int              state;
    int              timer;
    vec_t*           messagequeue;
    vectoranimator_t animator;
    element_t*       messageelement;

} notifierdata_t;

element_t* notifierelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    textstyle_t style,
    font_t*     font);

void notifierelement_queuemessage(element_t* element, str_t* message, input_t* input);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "cmd.c"
#include "textelement.c"
#include "zc_memory.c"
#include "zc_vector.c"

void notifierdata_dealloc(void* pointer);
void notifierelement_next(element_t* element, input_t* input);
void notifierelement_input(element_t* element, input_t* input);

/* create notifier element */

element_t* notifierelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    textstyle_t style,
    font_t*     font)
{

    element_t* element       = element_alloc("generic", name, x, y, width, height, NULL);
    element_t* messagelement = textelement_alloc("notifelement", 0, 0, width, height, NULL, NULL, font, style);

    notifierdata_t* data = CAL(sizeof(notifierdata_t), notifierdata_dealloc, NULL);

    data->messagequeue                = VNEW();
    data->messageelement              = messagelement;
    data->messageelement->translation = &data->animator.actual;

    element->data    = data;
    element->input   = notifierelement_input;
    element->exclude = 1;

    return element;
}

/* deletes notifier data */

void notifierdata_dealloc(void* pointer)
{
    notifierdata_t* data = pointer;
    REL(data->messagequeue);
}

/* queue notification */

void notifierelement_queuemessage(element_t* element, str_t* message, input_t* input)
{
    notifierdata_t* data = element->data;

    VADD(data->messagequeue, message);

    if (data->messagequeue->length == 1) notifierelement_next(element, input);
}

/* shows next notification */

void notifierelement_next(element_t* element, input_t* input)
{
    notifierdata_t* data = element->data;

    str_t* message = vec_head(data->messagequeue);

    textelement_settext(data->messageelement, input->font, input->cmdqueue, message);

    element_addsubelement(element, data->messageelement);

    cmdqueue_add(input->cmdqueue, "ui.addastimed", element, NULL);

    input->upload = 1;

    data->state = kNotifierStateAppear;
    data->timer = 0;

    vectoranimator_init(&data->animator, v2_init(0.0, -data->messageelement->height), v2_init(0.0, 0.0), input->ticks, input->ticks + 60, kAnimationTypeEaseOutExp);
    vectoranimator_step(&data->animator, input->ticks);
}

/* input event */

void notifierelement_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeTimer:
	{
	    notifierdata_t* data = element->data;

	    if (data->state == kNotifierStateAppear)
	    {
		if (data->animator.running == 1)
		{
		    vectoranimator_step(&data->animator, input->ticks);
		}
		else
		{
		    if (data->timer++ == 60) data->state = kNotifierStateIdle;
		}
	    }
	    else if (data->state == kNotifierStateIdle)
	    {
		if (data->timer++ == 120)
		{
		    data->state = kNotifierStateDisappear;
		    vectoranimator_init(&data->animator, data->animator.actual, v2_init(0.0, -data->messageelement->height), input->ticks, input->ticks + 60, kAnimationTypeEaseInExp);
		    vectoranimator_step(&data->animator, input->ticks);
		}
	    }
	    else if (data->state == kNotifierStateDisappear)
	    {
		if (data->animator.running == 1)
		{
		    vectoranimator_step(&data->animator, input->ticks);

		    if (data->animator.running == 0)
		    {
			vec_rem_at_index(data->messagequeue, 0);

			if (data->messagequeue->length > 0)
			{
			    notifierelement_next(element, input);
			}
			else
			{
			    element_removesubelement(element, data->messageelement);
			    input->upload = 1;
			    cmdqueue_add(input->cmdqueue, "ui.removeastimed", element, NULL);
			}
		    }
		}
	    }

	    input->render = 1;

	    break;
	}
	default:
	{
	    element_input(element, input);
	    break;
	}
    }
}

#endif
