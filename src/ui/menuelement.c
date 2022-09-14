

#include "menuelement.h"
#include "sliderelement.h"
#include "str_util.c"
#include "textelement.h"
#include "zc_memory.c"
#include "zc_string.c"

/* creates menuelement */

element_t* menuelement_alloc(
    char*       name,
    float       x,
    float       y,
    float       width,
    float       height,
    float       scale,
    textstyle_t text,
    font_t*     font,
    uint32_t    count,
    menuitem_t* items)
{
    element_t* element = solidelement_alloc("menu", x, y, width, height, 0x000000DD);

    element->data  = CAL(sizeof(menudata_t), NULL, NULL);
    element->input = menuelement_input;

    float position = 0.0;

    for (int index = 0; index < count; index++)
    {
	menuitem_t item        = items[index];
	element_t* itemelement = NULL;

	switch (item.type)
	{
	    case kMenuItemTypeGap:
	    {
		position += 5.0 * scale;
		break;
	    }
	    case kMenuItemTypeTitle:
	    {
		textstyle_t mystyle = text;
		mystyle.backcolor   = item.color;
		mystyle.textsize    = 120.0;

		str_t* label = str_frombytes(item.label);

		itemelement = textelement_alloc(
		    "menuitem",
		    0 * scale,
		    position,
		    350.0 * scale,
		    140.0,
		    label,
		    NULL,
		    font,
		    mystyle);

		REL(label);

		mystyle.textsize  = 20.0;
		mystyle.backcolor = 0;

		label = str_frombytes("by Milan Toth");

		element_t* homelement = textelement_alloc(
		    "homeitem",
		    205.0,
		    110.0,
		    120.0,
		    20.0,
		    label,
		    NULL,
		    font,
		    mystyle);

		REL(label);

		// homelement->autosize.keepxcenter = 1;

		element_addsubelement(itemelement, homelement);

		element_setaction(itemelement, "ontouchup", item.action);

		break;
	    }
	    case kMenuItemTypeLabel:
	    {
		str_t* label = str_frombytes(item.label);
		itemelement  = textelement_alloc("menuitem", 0 * scale, position, 300.0 * scale, 40.0 * scale, label, NULL, font, text);
		REL(label);

		break;
	    }
	    case kMenuItemTypeButton:
	    {
		textstyle_t mystyle = text;
		mystyle.backcolor   = item.color;
		str_t* label        = str_frombytes(item.label);
		itemelement         = textelement_alloc("menuitem", 0 * scale, position, 300.0 * scale, 40.0 * scale, label, NULL, font, mystyle);
		REL(label);

		element_setaction(itemelement, "ontouchup", item.action);

		break;
	    }
	    case kMenuItemTypeToggle:
	    {
		str_t* label = str_frombytes(item.label);
		itemelement  = textelement_alloc("menuitem", 0 * scale, position, 300.0 * scale, 40.0 * scale, label, NULL, font, text);
		REL(label);

		break;
	    }
	    case kMenuItemTypeSlider:
	    {
		text.backcolor         = 0;
		str_t* label           = str_frombytes(item.label);
		itemelement            = sliderelement_alloc("menuitem", 0 * scale, position, 300.0 * scale, 40.0 * scale, 0x00FF0088, item.color, 1, 1);
		element_t* textelement = textelement_alloc("textitem", 0, 0, 300.0 * scale, 40.0 * scale, label, NULL, font, text);

		element_addsubelement(itemelement, textelement);

		element_setaction(itemelement, "onslide", item.action);

		REL(label);
		REL(textelement);

		break;
	    }
	}

	if (itemelement != NULL)
	{
	    itemelement->autosize.keepxcenter = 1;
	    position += itemelement->height;

	    element_addsubelement(element, itemelement);
	}

	position += 3 * scale;
    }

    ((menudata_t*) (element->data))->innerheight = position;

    return element;
}

/* input event */

void menuelement_input(element_t* element, input_t* input)
{
    switch (input->type)
    {
	case kInputTypeResize:
	{
	    element_input(element, input);

	    if (element->subelements->length == 0) return;

	    menudata_t* data = element->data;

	    element_t* first = element->subelements->data[0];

	    float newtop = (input->floatb - data->innerheight) / 2.0;
	    float delta  = newtop - first->y;

	    for (int index = 0; index < element->subelements->length; index++)
	    {
		element_t* menuelement = element->subelements->data[index];
		menuelement->y += delta;
	    }

	    input->upload = 1;

	    break;
	}
	default:
	{
	    element_input(element, input);
	    break;
	}
    }
}
