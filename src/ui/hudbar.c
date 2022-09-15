#ifndef hudbar_h
#define hudbar_h

#include "element.c"

element_t* hudbarelement_alloc(
    char* name,
    float x,
    float y,
    float width,
    float height,
    float scale);

#endif /* hudbar_h */

#if __INCLUDE_LEVEL__ == 0

#include "defaults.c"
#include "sliderelement.c"
#include "str_util.c"
#include "textelement.c"
#include "zc_cstring.c"
#include "zc_memory.c"

element_t* hudbarelement_alloc(
    char* name,
    float x,
    float y,
    float width,
    float height,
    float scale)
{
    element_t* baseel;
    element_t* leftel;
    element_t* rghtel;

    baseel = solidelement_alloc(name, x, y, width, height, 0x00000022);

    leftel = sliderelement_alloc("bar", 0, 0, width / 2.0, height, 0x000000FF, 0x00AA00FF, 0, 0);

    rghtel = sliderelement_alloc("bar", width / 2.0, 0, width / 2.0, height, 0x00AA00FF, 0x000000FF, 0, 0);

    textstyle_t buttontexts = {0};

    buttontexts.font      = defaults.fontpath; // REL 0
    buttontexts.size      = 30.0 * defaults.scale;
    buttontexts.align     = TA_CENTER;
    buttontexts.margin    = (int) 5.0 * defaults.scale;
    buttontexts.textcolor = 0xFFFFFFFF;
    buttontexts.backcolor = 0xFFFFFF55;

    element_t* lefttext = textelement_alloc("lefttext", 0, 0, width / 2.0, height, str_frombytes("Health"), NULL, buttontexts);

    element_t* rghttext = textelement_alloc("righttext", 0, 0, width / 2.0, height, str_frombytes("Power"), NULL, buttontexts);

    element_addsubelement(baseel, leftel);

    element_addsubelement(baseel, rghtel);

    element_addsubelement(leftel, lefttext);

    element_addsubelement(rghtel, rghttext);

    REL(leftel);
    REL(rghtel);

    REL(baseel->type);
    baseel->type = cstr_new_cstring("hudbar");

    return baseel;
}

#endif
