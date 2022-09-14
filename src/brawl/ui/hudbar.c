
#ifndef hudbar_h
#define hudbar_h

#include "element.h"
#include "font.c"

element_t* hudbarelement_alloc(
    char*   name,
    float   x,
    float   y,
    float   width,
    float   height,
    float   scale,
    font_t* font);

#endif /* hudbar_h */

#if __INCLUDE_LEVEL__ == 0

#include "core/mtmem.c"
#include "hudbar.c"
#include "mtcstr.c"
#include "sliderelement.h"
#include "textelement.h"

element_t* hudbarelement_alloc(
    char*   name,
    float   x,
    float   y,
    float   width,
    float   height,
    float   scale,
    font_t* font)
{

    element_t* baseel;
    element_t* leftel;
    element_t* rghtel;

    baseel = solidelement_alloc(name, x, y, width, height, 0x00000022);

    leftel = sliderelement_alloc("bar", 0, 0, width / 2.0, height, 0x000000FF, 0x00AA00FF, 0, 0);

    rghtel = sliderelement_alloc("bar", width / 2.0, 0, width / 2.0, height, 0x00AA00FF, 0x000000FF, 0, 0);

    textstyle_t buttontexts =
	{
	    .align      = 1,
	    .editable   = 0,
	    .selectable = 0,
	    .multiline  = 0,
	    .autosize   = 0,
	    .uppercase  = 0,

	    .textsize   = 30.0 * scale,
	    .marginsize = 5.0 * scale,
	    .cursorsize = 0.0,

	    .textcolor = 0xFFFFFFFF,
	    .backcolor = 0xFFFFFF55};

    element_t* lefttext = textelement_alloc("lefttext", 0, 0, width / 2.0, height, mtstr_frombytes("Health"), NULL, font, buttontexts);

    element_t* rghttext = textelement_alloc("righttext", 0, 0, width / 2.0, height, mtstr_frombytes("Power"), NULL, font, buttontexts);

    element_addsubelement(baseel, leftel);

    element_addsubelement(baseel, rghtel);

    element_addsubelement(leftel, lefttext);

    element_addsubelement(rghtel, rghttext);

    mtmem_release(leftel);
    mtmem_release(rghtel);

    mtmem_release(baseel->type);
    baseel->type = mtcstr_fromcstring("hudbar");

    return baseel;
}

#endif
