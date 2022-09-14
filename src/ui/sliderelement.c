#ifndef sliderelement_h
#define sliderelement_h

#include "element.c"

typedef struct _sliderdata_t
{
    char editable;
    char animated;

    float ratio;
    float actual;

} sliderdata_t;

element_t* sliderelement_alloc(
    char*    name,
    float    x,
    float    y,
    float    width,
    float    height,
    uint32_t frontcolor,
    uint32_t backcolor,
    char     editable,
    char     animated);

void sliderelement_setslideaction(element_t* element, char* action);

void sliderelement_setratio(element_t* element, input_t* input, float ratio);

float sliderelement_getratio(element_t* element);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "textelement.c"
#include "zc_memory.c"
#include <float.h>

void sliderelement_input(element_t* element, input_t* input);

/* create slider element */

element_t* sliderelement_alloc(
    char*    name,
    float    x,
    float    y,
    float    width,
    float    height,
    uint32_t frontcolor,
    uint32_t backcolor,
    char     editable,
    char     animated)
{
    element_t* element = solidelement_alloc(name, x, y, width, height, backcolor);
    element_t* bar     = solidelement_alloc("bar", 0, 0, width, height, frontcolor);

    sliderdata_t* data = CAL(sizeof(sliderdata_t), NULL, NULL);

    element_addsubelement(element, bar);

    REL(bar);

    element_settype(element, "slider");

    element->data  = data;
    element->input = sliderelement_input;

    data->animated = animated;
    data->editable = editable;
    data->actual   = FLT_MAX;
    data->ratio    = 1.0;

    return element;
}

/* sets ratio of slider */

void sliderelement_setratio(element_t* element, input_t* input, float ratio)
{
    sliderdata_t* data = element->data;

    if (strcmp(element->type, "slider") != 0) return;

    if (data->ratio == ratio) return;

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    data->ratio = ratio;

    if (data->animated == 1)
    {
	cmdqueue_addonce(input->cmdqueue, "ui.addastimed", element, NULL);
    }
    else
    {
	element_t* bar = element->subelements->data[0];
	bar->width     = element->width * data->ratio;
	input->upload  = 1;
    }
}

/* returns ratio of slider */

float sliderelement_getratio(element_t* element)
{
    sliderdata_t* data = element->data;
    return data->ratio;
}

/* input event */

void sliderelement_input(element_t* element, input_t* input)
{
    sliderdata_t* data = element->data;
    switch (input->type)
    {
	case kInputTypeTimer:
	{
	    if (data->actual == FLT_MAX) data->actual = data->ratio;
	    data->actual += (data->ratio - data->actual) / 4.0;
	    element_t* bar = element->subelements->data[0];
	    bar->width     = element->width * data->actual;
	    input->upload  = 1;

	    if (data->animated == 1)
	    {
		if (fabs(data->ratio - data->actual) < 0.001) cmdqueue_add(input->cmdqueue, "ui.removeastimed", element, NULL);
	    }
	    break;
	}
	case kInputTypeTouchUp:
	case kInputTypeTouchDrag:
	{
	    sliderdata_t* data = element->data;
	    if (data->editable == 1)
	    {
		float ratio = (input->floata - element->finalx) / element->width;
		sliderelement_setratio(element, input, ratio);
		char* onslide = MGET(element->actions, "onslide");
		if (onslide != NULL) cmdqueue_add(input->cmdqueue, onslide, element, NULL);
	    }
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
