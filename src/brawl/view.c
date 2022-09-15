
#ifndef _view_header
#define _view_header

#include "base_metrics.c"
#include "element.c"
#include "input.h"
#include "ui.c"
#include "uirenderer.c"
#include "zc_map.c"

typedef struct _touch_t touch_t;
struct _touch_t
{
    char* id;
    float x;
    float y;
};

typedef struct _view_t view_t;
struct _view_t
{
    ui_t*         ui;
    element_t*    uibase;
    map_t*        uielements;
    uirenderer_t* uirenderer;

    base_metrics_t hero_metrics;
    cmdqueue_t*    cmdqueue;
};

void view_init(void);
void view_free(void);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "textelement.c"

#include "actor.c"
#include "bus.c"
#include "defaults.c"
#include "hudbar.c"
#include "menuelement.c"
#include "scene.c"
#include "sliderelement.c"
#include "zc_cstring.c"
#include "zc_text.c"
#include <linux/limits.h>

view_t view;

void view_onmessage(const char* name, void* data);
void view_updatemenu(void);
void view_updategenerator(void);
void view_setcontrolalpha(float alpha);
void view_init_menu(textstyle_t textstyle);
void view_init_controls(textstyle_t textstyle);
void view_init_hud(textstyle_t textstyle);
void view_init_generator(textstyle_t textstyle);
void view_init_wasted(textstyle_t textstyle);
void view_init_finished(textstyle_t textstyle);
void view_init_completed(textstyle_t textstyle);
void view_init_tip(textstyle_t textstyle);

/* init ui */

void view_init()
{
    bus_subscribe("VIEW", view_onmessage);

    text_init();

    /* textstyle_t textstyle = */
    /* 	{ */
    /* 	    .align      = 1, */
    /* 	    .editable   = 0, */
    /* 	    .selectable = 0, */
    /* 	    .multiline  = 0, */
    /* 	    .autosize   = 0, */
    /* 	    .uppercase  = 0, */

    /* 	    .textsize   = 30.0 * defaults.scale, */
    /* 	    .marginsize = 5.0 * defaults.scale, */
    /* 	    .cursorsize = 0.0, */

    /* 	    .textcolor = 0xFFFFFFFF, */
    /* 	    .backcolor = 0xFFFFFF55}; */

    textstyle_t textstyle = {0};

    textstyle.font   = defaults.fontpath; // REL 0
    textstyle.size   = 30.0 * defaults.scale;
    textstyle.align  = TA_CENTER;
    textstyle.margin = (int) 5.0 * defaults.scale;

    textstyle.textcolor = 0xFFFFFFFF;
    textstyle.backcolor = 0xFFFFFF55;

    view.ui           = ui_alloc(100.0);
    view.uibase       = element_alloc("base", "base", 0, 0, defaults.width, defaults.height, NULL);
    view.uielements   = MNEW();
    view.uirenderer   = uirenderer_alloc(defaults.width, defaults.height);
    view.cmdqueue     = cmdqueue_alloc();
    view.hero_metrics = base_metrics_default();

    view.uibase->autosize.fillx = 1;
    view.uibase->autosize.filly = 1;

    view_init_controls(textstyle);
    view_init_hud(textstyle);
    view_init_generator(textstyle);
    view_init_menu(textstyle);
    view_init_tip(textstyle);
    view_init_wasted(textstyle);
    view_init_finished(textstyle);
    view_init_completed(textstyle);

    view_setcontrolalpha(defaults.alpharatio);

    view_updategenerator();
    view_updatemenu();
}

void view_init_controls(textstyle_t textstyle)
{

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    textstyle_t bulletstyle = textstyle;
    bulletstyle.align       = 1;
    bulletstyle.margin      = 0;

    element_t* controlsbase = solidelement_alloc("controlsbase", 0, 0, width, height, 0x00000000);

    MPUT(view.uielements, "controlsbase", controlsbase);

    controlsbase->autosize.fillx = 1;
    controlsbase->autosize.filly = 1;

    char* hitpath   = cstr_new_format(PATH_MAX, "%s/hiticon.png", defaults.respath, NULL);
    char* kickpath  = cstr_new_format(PATH_MAX, "%s/kickicon.png", defaults.respath, NULL);
    char* blockpath = cstr_new_format(PATH_MAX, "%s/blockicon.png", defaults.respath, NULL);
    char* leftpath  = cstr_new_format(PATH_MAX, "%s/leftarrow.png", defaults.respath, NULL);
    char* rightpath = cstr_new_format(PATH_MAX, "%s/rightarrow.png", defaults.respath, NULL);
    char* uppath    = cstr_new_format(PATH_MAX, "%s/uparrow.png", defaults.respath, NULL);
    char* downpath  = cstr_new_format(PATH_MAX, "%s/downarrow.png", defaults.respath, NULL);
    char* shootpath = cstr_new_format(PATH_MAX, "%s/shooticon.png", defaults.respath, NULL);

    float btnsize   = 110.0 * scale;
    float icnsize   = 50.0 * scale;
    float icnmargin = (btnsize - icnsize) / 2.0;

    element_t* punchlabel = solidelement_alloc("punchlabel", 2 * scale, 464 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* punchicon  = imageelement_alloc("punchicon", icnmargin, icnmargin, icnsize, icnsize, hitpath);
    element_t* blocklabel = solidelement_alloc("blocklabel", 2 * scale, 576 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* blockicon  = imageelement_alloc("blockicon", icnmargin, icnmargin, icnsize, icnsize, blockpath);
    element_t* kicklabel  = solidelement_alloc("kicklabel", 2 * scale, 688 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* kickicon   = imageelement_alloc("kickicon", icnmargin, icnmargin, icnsize, icnsize, kickpath);
    element_t* shootlabel = solidelement_alloc("shootlabel", 114 * scale, 688 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* shooticon  = imageelement_alloc("shooticon", icnmargin, icnmargin, icnsize, icnsize, shootpath);
    element_t* leftlabel  = solidelement_alloc("leftlabel", 676 * scale, 576 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* lefticon   = imageelement_alloc("lefticon", icnmargin, icnmargin, icnsize, icnsize, leftpath);
    element_t* rightlabel = solidelement_alloc("rightlabel", 788 * scale, 576 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* righticon  = imageelement_alloc("righticon", icnmargin, icnmargin, icnsize, icnsize, rightpath);

    element_t* uplabel = solidelement_alloc("uplabel", 114 * scale, 576 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* upicon  = imageelement_alloc("upicon", icnmargin, icnmargin, icnsize, icnsize, uppath);

    element_t* downlabel = solidelement_alloc("downlabel", 732 * scale, 688 * scale, btnsize, btnsize, 0xFFFFFFFF);
    element_t* downicon  = imageelement_alloc("downicon", icnmargin, icnmargin, icnsize, icnsize, downpath);

    mem_release_each(hitpath, kickpath, blockpath, leftpath, rightpath, uppath, downpath, shootpath, NULL);

    element_setaction(punchlabel, "ontouchdown", "ctrldown");
    element_setaction(blocklabel, "ontouchdown", "ctrldown");
    element_setaction(kicklabel, "ontouchdown", "ctrldown");
    element_setaction(shootlabel, "ontouchdown", "ctrldown");
    element_setaction(leftlabel, "ontouchdown", "ctrldown");
    element_setaction(rightlabel, "ontouchdown", "ctrldown");
    element_setaction(uplabel, "ontouchdown", "ctrldown");
    element_setaction(downlabel, "ontouchdown", "ctrldown");

    element_setaction(punchlabel, "ontouchup", "ctrlup");
    element_setaction(blocklabel, "ontouchup", "ctrlup");
    element_setaction(kicklabel, "ontouchup", "ctrlup");
    element_setaction(shootlabel, "ontouchup", "ctrlup");
    element_setaction(leftlabel, "ontouchup", "ctrlup");
    element_setaction(rightlabel, "ontouchup", "ctrlup");
    element_setaction(uplabel, "ontouchup", "ctrlup");
    element_setaction(downlabel, "ontouchup", "ctrlup");

    punchlabel->autosize.bottommargin = 226 * scale;
    blocklabel->autosize.bottommargin = 114 * scale;

    kicklabel->autosize.bottommargin  = 2 * scale;
    shootlabel->autosize.bottommargin = 2 * scale;
    uplabel->autosize.bottommargin    = 114 * scale;

    downlabel->autosize.bottommargin = 2 * scale;
    downlabel->autosize.rightmargin  = 57 * scale;

    leftlabel->autosize.bottommargin = 114 * scale;
    leftlabel->autosize.rightmargin  = 114 * scale;

    rightlabel->autosize.bottommargin = 114 * scale;
    rightlabel->autosize.rightmargin  = 2 * scale;

    element_addsubelement(controlsbase, punchlabel);
    element_addsubelement(punchlabel, punchicon);
    element_addsubelement(controlsbase, blocklabel);
    element_addsubelement(blocklabel, blockicon);
    element_addsubelement(controlsbase, kicklabel);
    element_addsubelement(kicklabel, kickicon);
    element_addsubelement(controlsbase, leftlabel);
    element_addsubelement(shootlabel, shooticon);
    element_addsubelement(controlsbase, shootlabel);
    element_addsubelement(leftlabel, lefticon);
    element_addsubelement(controlsbase, rightlabel);
    element_addsubelement(rightlabel, righticon);
    element_addsubelement(controlsbase, uplabel);
    element_addsubelement(uplabel, upicon);
    element_addsubelement(controlsbase, downlabel);
    element_addsubelement(downlabel, downicon);

#ifdef OSX
    bulletstyle.backcolor = 0;
    element_t* punchtext  = textelement_alloc("punchtext", 0, 0, 40 * scale, 40 * scale, str_frombytes("'F'"), NULL, bulletstyle);
    element_t* blocktext  = textelement_alloc("blocktext", 0, 0, 40 * scale, 40 * scale, str_frombytes("'D'"), NULL, bulletstyle);
    element_t* kicktext   = textelement_alloc("kicktext", 0, 0, 40 * scale, 40 * scale, str_frombytes("'S'"), NULL, bulletstyle);
    element_t* shoottext  = textelement_alloc("shoottext", 0, 0, 40 * scale, 40 * scale, str_frombytes("'C'"), NULL, bulletstyle);

    element_addsubelement(punchlabel, punchtext);
    element_addsubelement(blocklabel, blocktext);
    element_addsubelement(kicklabel, kicktext);
    element_addsubelement(shootlabel, shoottext);
#endif

#if defined(IOS) || defined(ANDROID)
    element_addsubelement(view.uibase, controlsbase);
#endif
}

void view_init_hud(textstyle_t textstyle)
{

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    element_t* hudbar = hudbarelement_alloc("hudbar", width / 2.0 - 250.0 * scale, height - 50.0 * scale, 500.0 * scale, 40.0 * scale, scale);

    element_t* levelbar   = sliderelement_alloc("skillbar", 0.0 * scale, 0.0 * scale, 140 * scale, 40.0 * scale, 0x00AA00FF, 0x000000FF, 0, 0);
    element_t* leveltext  = textelement_alloc("skilltext", 0, 0, 140 * scale, 40 * scale, NULL, NULL, textstyle);
    element_t* menubutton = textelement_alloc("menubutton", width - 300.0 * scale, 0.0 * scale, 140 * scale, 40.0 * scale, str_frombytes("Menu"), NULL, textstyle);

    MPUT(view.uielements, "hudbar", hudbar);
    MPUT(view.uielements, "levelbar", levelbar);
    MPUT(view.uielements, "leveltext", leveltext);

    hudbar->autosize.bottommargin    = 0.1;
    hudbar->autosize.keepxcenter     = 1;
    menubutton->autosize.rightmargin = 0.1;

    ui_addtimed(view.ui, levelbar);
    ui_addtimed(view.ui, hudbar->subelements->data[0]);
    ui_addtimed(view.ui, hudbar->subelements->data[1]);

    element_setaction(menubutton, "ontouchup", "openmenu");

    element_addsubelement(levelbar, leveltext);

    // element_addsubelement( view.uibase , hudbar );
    element_addsubelement(view.uibase, levelbar);
    element_addsubelement(view.uibase, menubutton);

    mem_release_each(hudbar, levelbar, leveltext, menubutton, NULL);

    textstyle_t bulletstyle = textstyle;
    bulletstyle.align       = 1;
    bulletstyle.margin      = 0;

    element_t* bullettext = textelement_alloc("bullettext", hudbar->width / 2.0 - 20.0 * scale, 0, 40 * scale, 40 * scale, str_frombytes("0"), NULL, bulletstyle);

    MPUT(view.uielements, "bullettext", bullettext);
    element_addsubelement(hudbar, bullettext);
}

void view_init_generator(textstyle_t textstyle)
{

    textstyle_t redstyle   = textstyle;
    redstyle.backcolor     = 0xFF0000FF;
    textstyle_t bluestyle  = textstyle;
    bluestyle.backcolor    = 0x0000FFFF;
    textstyle_t greenstyle = textstyle;

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    element_t* paramelement = solidelement_alloc("paramsbase", 0, 0, width, height, 0x00000055);
    element_t* paramsbox    = solidelement_alloc("paramsbox", 0.0, 100.0 * scale, 606 * scale, 100 * scale, 0x00000055);

    paramelement->exclude        = 1;
    paramelement->autosize.fillx = 1;

    paramsbox->exclude              = 1;
    paramsbox->autosize.keepxcenter = 1;

    element_t* hitpowerbar  = sliderelement_alloc("hitpowerbar", 0 * scale, 2 * scale, 200 * scale, 40 * scale, 0x00AA00FF, 0x000000FF, 1, 1);
    element_t* hitpowertext = textelement_alloc("hitpowertext", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Hitpower"), NULL, textstyle);
    element_t* hitratebar   = sliderelement_alloc("hitratebar", 0 * scale, 44 * scale, 200 * scale, 40 * scale, 0x00AA00FF, 0x000000FF, 1, 1);
    element_t* hitratetext  = textelement_alloc("hitratetext", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Hitrate"), NULL, textstyle);
    element_t* heightbar    = sliderelement_alloc("heightbar", 202 * scale, 2 * scale, 200 * scale, 40 * scale, 0x00AA00FF, 0x000000FF, 1, 1);
    element_t* heighttext   = textelement_alloc("heighttext", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Height"), NULL, textstyle);
    element_t* staminabar   = sliderelement_alloc("staminabar", 404 * scale, 2 * scale, 200 * scale, 40 * scale, 0x00AA00FF, 0x000000FF, 1, 1);
    element_t* staminatext  = textelement_alloc("staminatext", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Stamina"), NULL, textstyle);
    element_t* speedbar     = sliderelement_alloc("speedbar", 404 * scale, 44 * scale, 200 * scale, 40 * scale, 0x00AA00FF, 0x000000FF, 1, 1);
    element_t* speedtext    = textelement_alloc("speedtext", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Speed"), NULL, textstyle);
    element_t* randombutton = textelement_alloc("randombutton", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Randomize"), NULL, greenstyle);
    element_t* startbutton  = textelement_alloc("startbutton", 0 * scale, 0 * scale, 200 * scale, 40 * scale, str_frombytes("Start Game"), NULL, redstyle);

    randombutton->autosize.keepxcenter  = 1;
    randombutton->autosize.bottommargin = 100.0 * scale;

    startbutton->autosize.keepxcenter  = 1;
    startbutton->autosize.bottommargin = 59.0 * scale;

    element_setaction(hitpowerbar, "onslide", "updatemetrics");
    element_setaction(hitratebar, "onslide", "updatemetrics");
    element_setaction(heightbar, "onslide", "updatemetrics");
    element_setaction(staminabar, "onslide", "updatemetrics");
    element_setaction(speedbar, "onslide", "updatemetrics");
    element_setaction(startbutton, "ontouchup", "nextlevel");
    element_setaction(randombutton, "ontouchup", "andommetrics");

    element_addsubelement(hitpowerbar, hitpowertext);
    element_addsubelement(hitratebar, hitratetext);
    element_addsubelement(heightbar, heighttext);
    element_addsubelement(staminabar, staminatext);
    element_addsubelement(speedbar, speedtext);

    element_addsubelement(paramsbox, hitpowerbar);
    element_addsubelement(paramsbox, hitratebar);
    element_addsubelement(paramsbox, heightbar);
    element_addsubelement(paramsbox, staminabar);
    element_addsubelement(paramsbox, speedbar);

    MPUT(view.uielements, "hitpowerbar", hitpowerbar);
    MPUT(view.uielements, "hitratebar", hitratebar);
    MPUT(view.uielements, "heightbar", heightbar);
    MPUT(view.uielements, "staminabar", staminabar);
    MPUT(view.uielements, "speedbar", speedbar);

    element_addsubelement(paramelement, paramsbox);
    element_addsubelement(paramelement, randombutton);
    element_addsubelement(paramelement, startbutton);

    element_addsubelement(view.uibase, paramelement);
    MPUT(view.uielements, "paramelement", paramelement);
}

void view_init_menu(textstyle_t textstyle)
{

    menuitem_t mainitems[13] =
	{
	    (menuitem_t){
		.type   = kMenuItemTypeTitle,
		.label  = "BRAWL",
		.color  = 0xFFFFFF11,
		.action = "homepage"},
	    (menuitem_t){
		.type = kMenuItemTypeGap,
	    },
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "Continue",
		.color  = 0xFFFFFF11,
		.action = "continue"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "New Game",
		.color  = 0xFFFFFF11,
		.action = "resetgame"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "Options",
		.color  = 0xFFFFFF11,
		.action = "options"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "Donate",
		.color  = 0xFFFFFF11,
		.action = "donate"}
#if !defined(IOS) && !defined(ANDROID)
	    ,
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "Exit",
		.color  = 0xFFFFFF11,
		.action = "exit"}
#endif
	};

    element_t* mainelement = menuelement_alloc(
	"menu",
	0,
	0,
	defaults.width,
	defaults.height,
	defaults.scale,
	textstyle,

	7,
	mainitems);

    mainelement->autosize.fillx = 1;
    mainelement->autosize.filly = 1;

    MPUT(view.uielements, "menuelement", mainelement);

    menuitem_t optsitems[13] =
    {
	(menuitem_t){
	    .type   = kMenuItemTypeSlider,
	    .label  = "Music Volume",
	    .color  = 0xFFFFFF11,
	    .action = "musicvol"},
	(menuitem_t){
	    .type   = kMenuItemTypeSlider,
	    .label  = "Sound Volume",
	    .color  = 0xFFFFFF11,
	    .action = "soundvol"},
#if defined(IOS) || defined(ANDROID)
	(menuitem_t){
	    .type   = kMenuItemTypeSlider,
	    .label  = "Controls Alpha",
	    .color  = 0xFFFFFF11,
	    .action = "ctrlalpha"},
#endif
	(menuitem_t){
	    .type   = kMenuItemTypeButton,
	    .label  = "Show/Hide Physics",
	    .color  = 0xFFFFFF11,
	    .action = "debug"},
#if !defined(IOS) && !defined(ANDROID)
	(menuitem_t){
	    .type   = kMenuItemTypeButton,
	    .label  = "Toggle Fullscreen",
	    .color  = 0xFFFFFF11,
	    .action = "fullscreen"},
#endif
	(menuitem_t){
	    .type   = kMenuItemTypeButton,
	    .label  = "Back",
	    .color  = 0xFFFFFF11,
	    .action = "back"}
    };

    element_t* optselement = menuelement_alloc(
	"opts",
	0,
	0,
	defaults.width,
	defaults.height,
	defaults.scale,
	textstyle,

	5,
	optsitems);

    optselement->autosize.fillx = 1;
    optselement->autosize.filly = 1;

    MPUT(view.uielements, "optselement", optselement);
}

void view_init_donations()
{

    menuitem_t dntsitems[4] =
	{
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = defaults.prices[0],
		.color  = 0xFFFFFF11,
		.action = "donate_a"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = defaults.prices[1],
		.color  = 0xFFFFFF11,
		.action = "donate_b"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = defaults.prices[2],
		.color  = 0xFFFFFF11,
		.action = "donate_c"},
	    (menuitem_t){
		.type   = kMenuItemTypeButton,
		.label  = "Back",
		.color  = 0xFFFFFF11,
		.action = "back"}

	};

    textstyle_t textstyle = {0};

    textstyle.font   = defaults.fontpath; // REL 0
    textstyle.size   = 30.0 * defaults.scale;
    textstyle.align  = TA_CENTER;
    textstyle.margin = (int) 5.0 * defaults.scale;

    textstyle.textcolor = 0xFFFFFFFF;
    textstyle.backcolor = 0xFFFFFF55;

    element_t* dntselement = menuelement_alloc(
	"dons",
	0,
	0,
	defaults.width,
	defaults.height,
	defaults.scale,
	textstyle,

	4,
	dntsitems);

    dntselement->autosize.fillx = 1;
    dntselement->autosize.filly = 1;

    MPUT(view.uielements, "dntselement", dntselement);

    input_t resizeinput  = {0};
    resizeinput.floata   = defaults.width;
    resizeinput.floatb   = defaults.height;
    resizeinput.type     = kInputTypeResize;
    resizeinput.ticks    = defaults.ticks;
    resizeinput.scale    = defaults.scale;
    resizeinput.cmdqueue = view.cmdqueue;

    if (dntselement != NULL) dntselement->input(dntselement, &resizeinput);
}

void view_init_tip(textstyle_t textstyle)
{

    textstyle.backcolor = 0x00000088;
    textstyle.multiline = 1;

    float width  = defaults.width;
    float height = defaults.height;

    str_t* str = str_frombytes("INFO");

    element_t* tipelement = solidelement_alloc("tipelement", 0, 0, width, height, 0);
    element_t* tiptext    = textelement_alloc("tiptext", 0, 100.0, width, 100, str, NULL, textstyle);

    REL(str);

    element_addsubelement(tipelement, tiptext);

    tipelement->autosize.fillx = 1;
    tipelement->autosize.filly = 1;

    tiptext->autosize.fillx = 1;

    MPUT(view.uielements, "tiptext", tiptext);
    MPUT(view.uielements, "tipelement", tipelement);

    mem_release_each(tipelement, tiptext, NULL);
}

void view_init_wasted(textstyle_t textstyle)
{

    textstyle_t greenstyle    = textstyle;
    greenstyle.backcolor      = 0x00AA00FF;
    textstyle_t lightredstyle = textstyle;
    lightredstyle.backcolor   = 0xFF000044;

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    str_t* infostr    = str_frombytes("Info");
    str_t* wastedstr  = str_frombytes("Wasted");
    str_t* restartstr = str_frombytes("Restart");

    element_t* wastedelement = solidelement_alloc("wastedelement", 0, 0, width, height, 0xAA0000CC);
    element_t* wastedinfo    = textelement_alloc("wastedinfo", 0, 50.0, width, 40, infostr, NULL, textstyle);
    element_t* wastedtext    = textelement_alloc("wastedtext", 0, 0, width, 150, wastedstr, NULL, lightredstyle);
    element_t* wastedbutton  = textelement_alloc("wastedbutton", 0, 0, width, 40.0, restartstr, NULL, greenstyle);

    element_setaction(wastedbutton, "ontouchup", "resetlevel");

    element_addsubelement(wastedelement, wastedinfo);
    element_addsubelement(wastedelement, wastedtext);
    element_addsubelement(wastedelement, wastedbutton);

    wastedelement->autosize.fillx = 1;
    wastedelement->autosize.filly = 1;

    wastedtext->autosize.fillx       = 1;
    wastedtext->autosize.keepycenter = 1;

    wastedinfo->autosize.fillx = 1;

    wastedbutton->autosize.fillx        = 1;
    wastedbutton->autosize.bottommargin = 50.0 * scale;

    MPUT(view.uielements, "wastedinfo", wastedinfo);
    MPUT(view.uielements, "wastedelement", wastedelement);

    mem_release_each(wastedelement, wastedinfo, wastedtext, wastedbutton, infostr, wastedstr, NULL);
}

void view_init_finished(textstyle_t textstyle)
{

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    textstyle_t greenstyle = textstyle;
    greenstyle.backcolor   = 0x00AA00FF;

    textstyle_t lightgreenstyle = textstyle;
    lightgreenstyle.backcolor   = 0x00AA0044;

    str_t* completedstr = str_frombytes("Level Completed");
    str_t* nextstr      = str_frombytes("Next Level");

    element_t* finishedelement = solidelement_alloc("finishedelement", 0, 0, width, height, 0x00FF00CC);
    element_t* finishedtext    = textelement_alloc("finishedtext", 0, 0, width, 150, completedstr, NULL, lightgreenstyle);
    element_t* finishedbutton  = textelement_alloc("finishedbutton", 0, 0, width, 40.0, nextstr, NULL, greenstyle);

    element_addsubelement(finishedelement, finishedtext);
    element_addsubelement(finishedelement, finishedbutton);

    element_setaction(finishedbutton, "ontouchup", "nextlevel");

    finishedelement->autosize.fillx = 1;
    finishedelement->autosize.filly = 1;

    finishedtext->autosize.fillx       = 1;
    finishedtext->autosize.keepycenter = 1;

    finishedbutton->autosize.fillx        = 1;
    finishedbutton->autosize.bottommargin = 50.0 * scale;

    MPUT(view.uielements, "finishedelement", finishedelement);

    mem_release_each(finishedelement, finishedtext, finishedbutton, completedstr, nextstr, NULL);
}

void view_init_completed(textstyle_t textstyle)
{

    float scale  = defaults.scale;
    float width  = defaults.width;
    float height = defaults.height;

    textstyle_t redstyle = textstyle;
    redstyle.backcolor   = 0xFF0000FF;

    textstyle_t bluestyle = textstyle;
    bluestyle.backcolor   = 0x0000FFFF;

    textstyle_t greenstyle = textstyle;
    greenstyle.backcolor   = 0x00AA00FF;

    textstyle_t lightredstyle = textstyle;
    lightredstyle.backcolor   = 0xFF000044;

    textstyle_t lightbluestyle = textstyle;
    lightbluestyle.backcolor   = 0x0000FF44;

    textstyle_t lightgreenstyle = textstyle;
    lightgreenstyle.backcolor   = 0x00AA0044;

    str_t* gamestr    = str_frombytes("Game Completed");
    str_t* resetstr   = str_frombytes("Reset and Restart");
    str_t* restartstr = str_frombytes("Restart");

    element_t* completedelement       = solidelement_alloc("completedelement", 0, 0, width, height, 0x0000FFCC);
    element_t* completedtext          = textelement_alloc("completedtext", 0, 0, width, 150, gamestr, NULL, lightbluestyle);
    element_t* completedrestartbutton = textelement_alloc("completedrestartbutton", 0, 0, width, 40.0, restartstr, NULL, bluestyle);
    element_t* completedresetbutton   = textelement_alloc("completedresetbutton", 0, 0, width, 40.0, resetstr, NULL, bluestyle);

    element_addsubelement(completedelement, completedtext);
    element_addsubelement(completedelement, completedresetbutton);
    element_addsubelement(completedelement, completedrestartbutton);

    element_setaction(completedresetbutton, "ontouchup", "resetgame");
    element_setaction(completedrestartbutton, "ontouchup", "restart");

    completedelement->autosize.fillx = 1;
    completedelement->autosize.filly = 1;

    completedtext->autosize.fillx       = 1;
    completedtext->autosize.keepycenter = 1;

    completedrestartbutton->autosize.fillx        = 1;
    completedrestartbutton->autosize.bottommargin = 100.0 * scale;

    completedresetbutton->autosize.fillx        = 1;
    completedresetbutton->autosize.bottommargin = 50.0 * scale;

    MPUT(view.uielements, "completedelement", completedelement);

    mem_release_each(completedelement, completedtext, completedresetbutton, completedrestartbutton, gamestr, restartstr, resetstr, NULL);
}

/* updates actor generator bars */

void view_updategenerator()
{
    input_t input  = {0};
    input.type     = kInputTypeUpdate;
    input.ticks    = defaults.ticks;
    input.scale    = defaults.scale;
    input.cmdqueue = view.cmdqueue;

    element_t* speedbar    = MGET(view.uielements, "speedbar");
    element_t* heightbar   = MGET(view.uielements, "heightbar");
    element_t* staminabar  = MGET(view.uielements, "staminabar");
    element_t* hitratebar  = MGET(view.uielements, "hitratebar");
    element_t* hitpowerbar = MGET(view.uielements, "hitpowerbar");

    sliderelement_setratio(speedbar, &input, view.hero_metrics.speed);
    sliderelement_setratio(heightbar, &input, view.hero_metrics.height);
    sliderelement_setratio(staminabar, &input, view.hero_metrics.stamina);
    sliderelement_setratio(hitratebar, &input, view.hero_metrics.hitrate);
    sliderelement_setratio(hitpowerbar, &input, view.hero_metrics.hitpower);
}

/* apply metrics to hero */

void view_applymetrics()
{

    view.hero_metrics.color_a = 0xFF0000FF;

    actor_t*      hero = scene.herogroup->actor;
    actor_skin_t* skin = scene.herogroup->skin;

    skin->acoords = 0.0;

    actor_updatemetrics(hero, view.hero_metrics);

    hero->health = hero->metrics.maxhealth;
    hero->power  = hero->metrics.maxpower;
}

/* randomize metrics */

void view_randomizemetrics()
{

    view.hero_metrics = base_metrics_random();

    input_t input  = {0};
    input.type     = kInputTypeUpdate;
    input.ticks    = defaults.ticks;
    input.scale    = defaults.scale;
    input.cmdqueue = view.cmdqueue;

    view_updategenerator();
    view_applymetrics();
}

/* metric bar changed on ui, update hero metrics */

void view_updatemetrics(element_t* element)
{

    float           value   = sliderelement_getratio(element);
    base_metrics_t* metrics = &view.hero_metrics;

    float heightchange   = 0.0;
    float speedchange    = 0.0;
    float staminachange  = 0.0;
    float hitratechange  = 0.0;
    float hitpowerchange = 0.0;

    if (strcmp(element->name, "heightbar") == 0) heightchange = value - metrics->height;
    if (strcmp(element->name, "hitpowerbar") == 0) hitpowerchange = value - metrics->hitpower;
    if (strcmp(element->name, "hitratebar") == 0) hitratechange = value - metrics->hitrate;
    if (strcmp(element->name, "staminabar") == 0) staminachange = value - metrics->stamina;
    if (strcmp(element->name, "speedbar") == 0) speedchange = value - metrics->speed;

    base_metrics_update(metrics, heightchange, hitpowerchange, hitratechange, staminachange, speedchange);

    input_t input  = {0};
    input.type     = kInputTypeUpdate;
    input.ticks    = defaults.ticks;
    input.scale    = defaults.scale;
    input.cmdqueue = view.cmdqueue;

    view_updategenerator();

    view_applymetrics();
}

// cleanup

void view_free(void)
{

    REL(view.uibase);
}

/* add text bubble over actor */

void view_addbubble(input_t* input)
{
    // element_t* textbubble = textelement_alloc( "textbubble", 0, 0 , 300, 50, greenstyle,  str_frombytes("TEXT BUBBLE"), NULL, 0.0 );
}

/* add level bar over actor */

void view_addlevelbar(input_t* input)
{
}

/* resize elements */

void view_resize()
{

    element_t* menuel;
    element_t* wstdel;
    element_t* fnshel;
    element_t* cmplel;
    element_t* optsel;
    element_t* tipsel;
    element_t* parmel;
    element_t* dntsel;
    element_t* hudbar;

    hudbar = MGET(view.uielements, "hudbar");
    tipsel = MGET(view.uielements, "tipelement");
    menuel = MGET(view.uielements, "menuelement");
    optsel = MGET(view.uielements, "optselement");
    dntsel = MGET(view.uielements, "dntselement");
    parmel = MGET(view.uielements, "paramelement");
    wstdel = MGET(view.uielements, "wastedelement");
    fnshel = MGET(view.uielements, "finishedelement");
    cmplel = MGET(view.uielements, "completedelement");

    input_t resizeinput  = {0};
    resizeinput.floata   = defaults.width;
    resizeinput.floatb   = defaults.height;
    resizeinput.type     = kInputTypeResize;
    resizeinput.ticks    = defaults.ticks;
    resizeinput.scale    = defaults.scale;
    resizeinput.cmdqueue = view.cmdqueue;

    view.uibase->input(view.uibase, &resizeinput);

    hudbar->input(hudbar, &resizeinput);
    tipsel->input(tipsel, &resizeinput);
    menuel->input(menuel, &resizeinput);
    optsel->input(optsel, &resizeinput);
    wstdel->input(wstdel, &resizeinput);
    fnshel->input(fnshel, &resizeinput);
    cmplel->input(cmplel, &resizeinput);
    parmel->input(parmel, &resizeinput);
    if (dntsel != NULL) dntsel->input(dntsel, &resizeinput);
}

// update menu

void view_updatemenu()
{
    input_t input  = {0};
    input.type     = kInputTypeUpdate;
    input.ticks    = defaults.ticks;
    input.scale    = defaults.scale;
    input.cmdqueue = view.cmdqueue;

    element_t* optselement = MGET(view.uielements, "optselement");

    element_t* musicelement    = optselement->subelements->data[0];
    element_t* soundelement    = optselement->subelements->data[1];
    element_t* zoomelement     = optselement->subelements->data[2];
    element_t* controlselement = optselement->subelements->data[3];

    sliderelement_setratio(musicelement, &input, defaults.musicvolume);

    sliderelement_setratio(soundelement, &input, defaults.soundvolume);

    sliderelement_setratio(zoomelement, &input, defaults.zoomratio);

    sliderelement_setratio(controlselement, &input, defaults.alpharatio);
}

/* updates hud */

void view_timer()
{

    input_t input  = {0};
    input.type     = kInputTypeTimer;
    input.ticks    = defaults.ticks;
    input.scale    = defaults.scale;
    input.cmdqueue = view.cmdqueue;

    ui_timer(view.ui, &input);

    /* update actor huds */

    for (int index = 0;
	 index < scene.actors->length;
	 index++)
    {

	actor_group_t* group = scene.actors->data[index];
	actor_t*       actor = group->actor;

	/* update hud & bubble */

	element_t* hud = group->hud;

	if (actor->points.hip.x > scene.lefttop2world.x - 50.0 &&
	    actor->points.hip.x < scene.rightbtm2world.x + 50.0 &&
	    actor->points.hip.y < scene.lefttop2world.y - 50.0 &&
	    actor->points.hip.y > scene.rightbtm2world.y + 50.0)
	{

	    v4_t point = m4_world_to_screen_coords(scene.matrix, v4_init(actor->points.head.x, actor->points.head.y, 0, 0), defaults.width, defaults.height);

	    if (index > 0)
	    {
		hud->x += (point.x - hud->width / 2.0 - hud->x) / 4.0;
		hud->y += ((defaults.height - point.y - 80.0 * defaults.scale) - hud->y) / 4.0;

		if (hud->exclude == 1 && actor->state != kActorStateDeath)
		{
		    hud->exclude = 0;
		    element_setposition(hud, point.x - hud->width / 2.0, defaults.height - point.y - hud->height * 1.5);
		    element_addsubelement(view.uibase, hud);
		}
	    }

	    element_t* healthbar = hud->subelements->data[0];
	    sliderelement_setratio(healthbar, &input, actor->health / actor->metrics.maxhealth);

	    if (group->currenttext != NULL && group->bubble == NULL)
	    {

		textstyle_t textstyle = {0};

		textstyle.font   = defaults.fontpath; // REL 0
		textstyle.size   = 20.0 * defaults.scale;
		textstyle.align  = TA_CENTER;
		textstyle.margin = (int) 5.0 * defaults.scale;

		textstyle.textcolor = 0x000000FF;
		textstyle.backcolor = 0xFFFFFF88;

		element_t* element = textelement_alloc("bubble", 0, 0, 100.0, 20.0, group->currenttext, NULL, textstyle);

		actor_group_setbubble(group, element);

		REL(group->currenttext);
		group->currenttext = NULL;
	    }

	    if (group->bubble != NULL)
	    {
		element_t* bubble = group->bubble;

		float delta = 0.0;
		if (actor->facing < 0) delta = -bubble->width - 30.0 * defaults.scale;
		else delta = 30.0 * defaults.scale;
		element_setposition(bubble, point.x + delta, defaults.height - point.y);

		if (group->removetime == 0)
		{

		    group->removetime = input.ticks + 100;
		    element_addsubelement(view.uibase, bubble);
		}
		else if (group->removetime == input.ticks)
		{

		    element_removesubelement(view.uibase, bubble);
		    group->removetime = 0;

		    actor_group_setbubble(group, NULL);
		}
	    }
	}
	else if (hud->exclude == 0)
	{
	    hud->exclude = 1;
	    element_removesubelement(view.uibase, hud);
	}
    }

    /* update meters */

    actor_t* actor = scene.herogroup->actor;

    element_t* hudbar   = MGET(view.uielements, "hudbar");
    element_t* levelbar = MGET(view.uielements, "levelbar");

    element_t* healthbar = hudbar->subelements->data[0];
    element_t* powerbar  = hudbar->subelements->data[1];

    float helthr = 1.0 - actor->health / actor->metrics.maxhealth;
    float powerr = actor->power / actor->metrics.maxpower;
    float levelr = (float) (actor->metrics.xp) / 1000.0;

    sliderelement_setratio(healthbar, &input, helthr);

    sliderelement_setratio(powerbar, &input, powerr);

    sliderelement_setratio(levelbar, &input, levelr);
}

/* updates skill */

void view_updateskill()
{

    actor_t* actor = scene.herogroup->actor;

    int skill = actor->metrics.level;

    element_t* leveltext = MGET(view.uielements, "leveltext");

    char text[3];

    snprintf(text, 3, "%i", skill);

    char*  textcs = cstr_new_format(100, "Skill %s", text, NULL);
    str_t* texts  = str_new();
    str_add_bytearray(texts, textcs);

    textelement_settext(leveltext, view.cmdqueue, texts);

    REL(texts);
}

/* set alpha of controls */

void view_setcontrolalpha(float alpha)
{

    defaults.alpharatio = alpha;
    defaults_save();

    uint8_t  newalpha = (uint8_t) ((float) 125 * alpha);
    uint32_t color    = (0xFFFFFF << 8) | newalpha;

    element_t* controlbase = MGET(view.uielements, "controlsbase");

    for (int index = 0;
	 index < controlbase->subelements->length;
	 index++)
    {

	element_t* element = controlbase->subelements->data[index];

	solidelement_setcolor(element, color);
    }
}

/* shows given elements */

void view_showelement(char* name)
{

    element_t* element = MGET(view.uielements, name);

    if (element != NULL)
    {
	element_addsubelement(view.uibase, element);
    }
}

/* hides given elements */

void view_hideelement(char* name)
{

    element_t* element = MGET(view.uielements, name);

    if (element != NULL)
    {
	element_removesubelement(view.uibase, element);
    }
}

/* shows wasted */

void view_showwasted(str_t* text)
{

    element_t* textel;
    element_t* baseel;

    textel = MGET(view.uielements, "wastedinfo");
    baseel = MGET(view.uielements, "wastedelement");

    textelement_settext(textel, view.cmdqueue, text);

    if (baseel != NULL)
    {
	element_addsubelement(view.uibase, baseel);
    }
}

/* shows tip */

void view_showtip(char* text)
{

    element_t* textel;
    element_t* baseel;

    textel = MGET(view.uielements, "tiptext");
    baseel = MGET(view.uielements, "tipelement");

    if (strcmp(text, "Pivot_i1") == 0)
    {
	text = "Use the ARROWS to move left, right, jump and squat.\nPress SPACE to run.";
    }
    else if (strcmp(text, "Pivot_i2") == 0)
    {
	text = "To protect yourself use F to punch, D to kick and S to block.";
    }
    else if (strcmp(text, "Pivot_i3") == 0)
    {
	text = "Press DOWN to go down at a fork.";
    }
    else if (strcmp(text, "Pivot_i4") == 0)
    {
	text = "Press DOWN to pick up a dead body. Run and press F to throw it at somebody";
    }
    else if (strcmp(text, "Pivot_i5") == 0)
    {
	text = "Press DOWN to pick up a gun. Press C to fire it.";
    }
    else if (strcmp(text, "PlsGive") == 0)
    {
	text = "To pick up stuff after the third level please donate from the main menu.";
    }

    str_t* str = str_frombytes(text);

    textelement_settext(textel, view.cmdqueue, str);

    REL(str);

    if (baseel != NULL)
    {
	element_addsubelement(view.uibase, baseel);
    }

    cmdqueue_delay(view.cmdqueue, "hidetip", NULL, NULL, defaults.ticks + 300);
}
/* activate ui */

void view_activate()
{

    vec_reset(view.ui->visible);

    element_collectelements(view.uibase, v2_init(0.0, 0.0), view.ui->visible);

    /* add to renderer */

    int success = 0;
    uirenderer_reset(view.uirenderer);

    for (int index = 1;
	 index < view.ui->visible->length;
	 index++)
    {

	element_t* element = view.ui->visible->data[index];
	success            = uirenderer_addelement(view.uirenderer, element);
	if (success == -1) break;
    }

    /* if tiled texture is full, re-add everything */

    if (success == -1)
    {
	printf("RESET RENDERER\n");
	uirenderer_reset(view.uirenderer);

	for (int index = 1;
	     index < view.ui->visible->length;
	     index++)
	{

	    element_t* element = view.ui->visible->data[index];
	    uirenderer_addelement(view.uirenderer, element);
	}
    }
}

void view_removebubbles(void)
{

    for (int index = 0;
	 index < scene.actors->length;
	 index++)
    {

	actor_group_t* group = scene.actors->data[index];

	if (group->bubble != NULL) element_removesubelement(view.uibase, group->bubble);

	if (group->hud != NULL) element_removesubelement(view.uibase, group->hud);
    }
}

/* control button event from ui */

void view_ctrldown(input_t* input, element_t* element)
{
    if (strcmp(element->name, "leftlabel") == 0)
    {
	scene.control_state.left_pressed = 1;
	scene.control_state.run_pressed  = 1;
    }
    else if (strcmp(element->name, "rightlabel") == 0)
    {
	scene.control_state.right_pressed = 1;
	scene.control_state.run_pressed   = 1;
    }
    else if (strcmp(element->name, "uplabel") == 0)
    {
	scene.control_state.jump_pressed = 1;
    }
    else if (strcmp(element->name, "downlabel") == 0) scene.control_state.squat_pressed = 1;
    else if (strcmp(element->name, "kicklabel") == 0) scene.control_state.kick_pressed = 1;
    else if (strcmp(element->name, "blocklabel") == 0) scene.control_state.block_pressed = 1;
    else if (strcmp(element->name, "punchlabel") == 0) scene.control_state.punch_pressed = 1;
    else if (strcmp(element->name, "shootlabel") == 0) scene.control_state.shoot_pressed = 1;

    input->render = 0;
}

/* key release event */

void view_ctrlup(input_t* input, element_t* element)
{

    if (strcmp(element->name, "leftlabel") == 0) scene.control_state.left_pressed = 0;
    else if (strcmp(element->name, "rightlabel") == 0) scene.control_state.right_pressed = 0;
    else if (strcmp(element->name, "uplabel") == 0) scene.control_state.jump_pressed = 0;
    else if (strcmp(element->name, "downlabel") == 0) scene.control_state.squat_pressed = 0;
    else if (strcmp(element->name, "kicklabel") == 0) scene.control_state.kick_pressed = 0;
    else if (strcmp(element->name, "blocklabel") == 0) scene.control_state.block_pressed = 0;
    else if (strcmp(element->name, "punchlabel") == 0) scene.control_state.punch_pressed = 0;
    else if (strcmp(element->name, "shootlabel") == 0) scene.control_state.shoot_pressed = 0;

    if (scene.control_state.left_pressed == 0 &&
	scene.control_state.right_pressed == 0)
    {
	scene.control_state.run_pressed = 0;
    }

    input->render = 0;
}

/* sets music volume */

void scene_setmusicvolume(input_t* input, element_t* element)
{

    float ratio = sliderelement_getratio(element);
    bus_notify("SND", "MUSICVOLUME", &ratio);
}

void scene_setsoundvolume(input_t* input, element_t* element)
{

    float ratio = sliderelement_getratio(element);
    bus_notify("SND", "SOUNDVOLUME", &ratio);
}

void scene_setzoomratio(input_t* input, element_t* element)
{
    defaults.zoomratio = sliderelement_getratio(element);
    defaults_save();

    scene.zoom.ratio = 0.5 + sliderelement_getratio(element);
}

void view_input(input_t* input)
{
    static char inloop = 0;

    input->ticks    = defaults.ticks;
    input->scale    = defaults.scale;
    input->cmdqueue = view.cmdqueue;

    switch (input->type)
    {
	case kInputTypeTouchDrag: ui_touch_drag(view.ui, input); break;
	case kInputTypeTouchDown: ui_touch_down(view.ui, input); break;
	case kInputTypeTouchUp: ui_touch_up(view.ui, input); break;
    }

    if (view.cmdqueue->commands->length > 0 && inloop == 0)
    {
	inloop = 1;
	for (int index = 0;
	     index < view.cmdqueue->commands->length;
	     index++)
	{
	    cmd_t* command = (cmd_t*) view.cmdqueue->commands->data[index];

	    // printf( "ACTION %s\n" , command->name );

	    if (strcmp(command->name, "ui.addastimed") == 0)
	    {
		ui_addtimed(view.ui, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "ui.removeastimed") == 0)
	    {
		ui_removetimed(view.ui, (element_t*) command->data);
	    }
	    // main menu
	    else if (strcmp(command->name, "homepage") == 0)
	    {
		bus_notify("CTL", "HOMEPAGE", NULL);
	    }
	    else if (strcmp(command->name, "continue") == 0)
	    {
		if (defaults.sceneindex > 0) view_showelement((char*) "hudbar");
		else view_hideelement((char*) "hudbar");
		view_hideelement((char*) "menuelement");
	    }
	    else if (strcmp(command->name, "resetgame") == 0)
	    {
		view_hideelement((char*) "hudbar");
		view_hideelement("menuelement");
		view_hideelement("finishedelement");
		view_hideelement("completedelement");
		view_showelement("paramelement");
		view_randomizemetrics();
		view_removebubbles();
		bus_notify("CTL", "RESETGAME", NULL);
	    }
	    else if (strcmp(command->name, "options") == 0)
	    {
		view_hideelement((char*) "menuelement");
		view_showelement((char*) "optselement");
	    }
	    else if (strcmp(command->name, "donate") == 0)
	    {
		if (defaults.prices_arrived == 1)
		{
#ifdef RASPBERRY
		    bus_notify("CTL", "DONATE", defaults.prices[0]);
		    view_hideelement((char*) "optselement");
		    view_hideelement((char*) "dntselement");
		    view_showelement((char*) "menuelement");
#else
		    element_t* dntselement = MGET(view.uielements, "donselement");
		    if (dntselement == NULL) view_init_donations();
		    view_hideelement((char*) "menuelement");
		    view_showelement((char*) "dntselement");
#endif
		}
	    }
	    else if (strcmp(command->name, "donate_a") == 0)
	    {
		bus_notify("CTL", "DONATE", defaults.prices[0]);
		view_hideelement((char*) "optselement");
		view_hideelement((char*) "dntselement");
		view_showelement((char*) "menuelement");
	    }
	    else if (strcmp(command->name, "donate_b") == 0)
	    {
		bus_notify("CTL", "DONATE", defaults.prices[1]);
		view_hideelement((char*) "optselement");
		view_hideelement((char*) "dntselement");
		view_showelement((char*) "menuelement");
	    }
	    else if (strcmp(command->name, "donate_c") == 0)
	    {
		bus_notify("CTL", "DONATE", defaults.prices[2]);
		view_hideelement((char*) "optselement");
		view_hideelement((char*) "dntselement");
		view_showelement((char*) "menuelement");
	    }
	    else if (strcmp(command->name, "exit") == 0)
	    {
		bus_notify("CTL", "EXIT", NULL);
	    }
	    // options
	    else if (strcmp(command->name, "soundvol") == 0)
	    {
		scene_setsoundvolume(input, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "musicvol") == 0)
	    {
		scene_setmusicvolume(input, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "setzoom") == 0)
	    {
		scene_setzoomratio(input, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "ctrlalpha") == 0)
	    {
		view_setcontrolalpha(sliderelement_getratio((element_t*) command->data));
	    }
	    else if (strcmp(command->name, "debug") == 0)
	    {
		defaults.debug_mode = 1 - defaults.debug_mode;
	    }
	    else if (strcmp(command->name, "fullscreen") == 0)
	    {
		bus_notify("CTL", "FULLSCREEN", NULL);
	    }
	    else if (strcmp(command->name, "back") == 0)
	    {
		view_hideelement((char*) "optselement");
		view_hideelement((char*) "dntselement");
		view_showelement((char*) "menuelement");
	    }
	    // game ui
	    else if (strcmp(command->name, "hidetip") == 0)
	    {
		view_hideelement((char*) "tipelement");
		view_activate();
	    }
	    else if (strcmp(command->name, "openmenu") == 0)
	    {
		view_showelement((char*) "menuelement");
	    }
	    else if (strcmp(command->name, "ctrldown") == 0)
	    {
		view_ctrldown(input, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "ctrlup") == 0)
	    {
		view_ctrlup(input, (element_t*) command->data);
	    }
	    else if (strcmp(command->name, "restart") == 0)
	    {
		view_hideelement((char*) "finishedelement");
		view_hideelement((char*) "completedelement");
		view_removebubbles();
		view_activate();
		bus_notify("CTL", "RESTARTGAME", NULL);
	    }
	    else if (strcmp(command->name, "resetlevel") == 0)
	    {
		view_hideelement((char*) "menuelement");
		view_hideelement((char*) "wastedelement");
		view_hideelement((char*) "finishedelement");
		view_removebubbles();
		view_activate();
		bus_notify("CTL", "RESETLEVEL", NULL);
	    }
	    else if (strcmp(command->name, "nextlevel") == 0)
	    {
		view_showelement((char*) "hudbar");
		view_hideelement((char*) "paramelement");
		view_hideelement((char*) "finishedelement");
		view_removebubbles();
		view_activate();
		bus_notify("CTL", "NEXTLEVEL", NULL);
	    }
	    // actor generator
	    else if (strcmp(command->name, "updatemetrics") == 0)
	    {
		view_updatemetrics(command->data);
	    }
	    else if (strcmp(command->name, "andommetrics") == 0)
	    {
		view_randomizemetrics();
	    }
	}

	cmdqueue_reset(view.cmdqueue);
	inloop = 0;
    }
}

void view_onmessage(const char* name, void* data)
{

    if (strcmp(name, "UPDATE") == 0)
    {

	cmdqueue_timer(view.cmdqueue, defaults.ticks);
	view_timer();

	input_t input = {0};
	input.type    = kInputTypeTimer;

	view_input(&input);

	view_activate();
    }
    else if (strcmp(name, "RENDER") == 0)
    {

	uirenderer_draw(view.uirenderer);
    }
    else if (strcmp(name, "TOUCHDOWN") == 0)
    {

	touch_t touch = *(touch_t*) data;

	input_t input = {0};
	input.type    = kInputTypeTouchDown;
	input.stringa = touch.id;
	input.floata  = touch.x;
	input.floatb  = touch.y;

	view_input(&input);
    }
    else if (strcmp(name, "TOUCHUP") == 0)
    {

	touch_t touch = *(touch_t*) data;

	input_t input = {0};
	input.type    = kInputTypeTouchUp;
	input.stringa = touch.id;
	input.floata  = touch.x;
	input.floatb  = touch.y;

	view_input(&input);
    }
    else if (strcmp(name, "TOUCHMOVE") == 0)
    {

	touch_t touch = *(touch_t*) data;

	input_t input = {0};
	input.type    = kInputTypeTouchDrag;
	input.stringa = touch.id;
	input.floata  = touch.x;
	input.floatb  = touch.y;

	view_input(&input);
    }
    else if (strcmp(name, "RESIZE") == 0)
    {

	m4_t perspective_matrix             = m4_defaultortho(0.0, defaults.width, -defaults.height, 0.0, 0.0, 1.0);
	perspective_matrix                  = m4_scale(perspective_matrix, 1.0, -1.0, 1.0);
	view.uirenderer->perspective_matrix = perspective_matrix;

	uirenderer_resize(view.uirenderer, defaults.width, defaults.height);

	view_resize();
	view_activate();
    }
    else if (strcmp(name, "SHOWELEMENT") == 0)
    {

	view_showelement((char*) data);
	view_activate();
    }
    else if (strcmp(name, "HIDEELEMENT") == 0)
    {

	view_hideelement((char*) data);
	view_activate();
    }
    else if (strcmp(name, "SHOWWASTED") == 0)
    {

	view_showwasted((str_t*) data);
	view_activate();
    }
    else if (strcmp(name, "SHOWTIP") == 0)
    {

	view_showtip((char*) data);
	view_activate();
    }
    else if (strcmp(name, "UPDATESKILL") == 0)
    {

	view_updateskill();
    }
    else if (strcmp(name, "REMOVEHUD") == 0)
    {

	actor_group_t* group = data;
	element_t*     hud   = group->hud;

	hud->exclude = 1;
	element_removesubelement(view.uibase, hud);
    }
    else if (strcmp(name, "UPDATEBULLETS") == 0)
    {

	actor_group_t* group = data;
	gun_t*         gun   = group->gun;

	element_t* bullettext = MGET(view.uielements, "bullettext");

	char intstring[2] = {0};

	snprintf(intstring, 2, "%i", gun->bullets);

	str_t* text = str_frombytes(intstring);

	textelement_settext(bullettext, view.cmdqueue, text);

	REL(text);
    }
}

#endif
