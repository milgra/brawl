
#ifndef actor_hud_element_h
#define actor_hud_element_h

#include "sliderelement.c"

element_t* actor_hud_element_alloc(char* name, uint32_t color, float scale, font_t* font);

#endif /* actor_hud_element_h */

#if __INCLUDE_LEVEL__ == 0

#include "actor_hud_element.c"
#include "str_util.c"
#include "text.c"
#include "textelement.c"
#include "zc_cstring.c"
#include "zc_memory.c"

element_t* actor_hud_element_alloc(char* name, uint32_t color, float scale, font_t* font)
{
    textstyle_t buttontexts =
	{
	    .align      = 1,
	    .editable   = 0,
	    .selectable = 0,
	    .multiline  = 1,
	    .autosize   = 0,
	    .uppercase  = 0,

	    .textsize   = 20.0 * scale,
	    .marginsize = 0.0 * scale,
	    .cursorsize = 0.0,

	    .textcolor = 0xFFFFFFFF,
	    .backcolor = 0};

    str_t* namestring = str_frombytes(name);

    element_t* element   = solidelement_alloc(name, 0, 0, 70.0 * scale, 20.0 * scale, 0);
    element_t* nametext  = textelement_alloc("righttext", 0, 0, 70.0 * scale, 20.0 * scale, namestring, NULL, font, buttontexts);
    element_t* healthbar = sliderelement_alloc("bar", 0, 0, 70.0 * scale, 20.0 * scale, (color & 0xFFFFFF00) | 0xAA, 0x000000AA, 0, 0);

    element_addsubelement(element, healthbar);
    element_addsubelement(element, nametext);

    nametext->x = element->width / 2.0 - nametext->width / 2.0;

    mem_release_each(healthbar, namestring, nametext, NULL);

    REL(element->type);
    element->type    = cstr_new_cstring("hudelement");
    element->exclude = 1;

    return element;
}

#endif
