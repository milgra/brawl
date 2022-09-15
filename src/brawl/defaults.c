
#ifndef defaults_h
#define defaults_h

#include "zc_cstring.c"
#include "zc_vec2.c"
#include <linux/limits.h>
#include <stdlib.h>

typedef struct _defaults_t
{
    char debug_mode;

    int effects_level;
    int sceneindex;
    int hudvisible;

    float musicvolume;
    float soundvolume;
    float zoomratio;
    float alpharatio;

    // runtime

    v2_t     gravity;
    uint32_t ticks;

    float scale;
    float width;
    float height;

    char* libpath;
    char* respath;
    char* fontpath;
} defaults_t;

extern defaults_t defaults;

void defaults_init(char* libpath, char* respath);
void defaults_free(void);
void defaults_save(void);
void defaults_reset(void);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "settings.c"

defaults_t defaults = {0};

void defaults_init(char* libpath, char* respath)
{
    defaults.libpath  = cstr_new_cstring(libpath);
    defaults.respath  = cstr_new_cstring(respath);
    defaults.fontpath = cstr_new_format(PATH_MAX, "%s/Impact.ttf", defaults.respath);

    printf("FONTPATH %s\n", defaults.fontpath);

    settings_init(libpath, (char*) "brawl.state");

    int inited = settings_getint("initialized");

    if (inited == 0)
    {
	settings_setint("initialized", 1);

	settings_setint("sceneindex", 0);
	settings_setint("hudvisible", 1);

	settings_setfloat("musicvolume", .5);
	settings_setfloat("soundvolume", .5);
	settings_setfloat("zoomratio", .5);
	settings_setfloat("alpharatio", .5);

	settings_setint("effects_level", 2);
    }

    defaults.effects_level = settings_getint("effects_level");
    defaults.sceneindex    = settings_getint("sceneindex");
    defaults.hudvisible    = settings_getint("hudvisible");

    defaults.musicvolume = settings_getfloat("musicvolume");
    defaults.soundvolume = settings_getfloat("soundvolume");
    defaults.zoomratio   = settings_getfloat("zoomration");
    defaults.alpharatio  = settings_getfloat("alpharatio");

    defaults.ticks      = 0;
    defaults.debug_mode = 0;
    defaults.gravity    = v2_init(0.0, -0.3);
}

void defaults_free()
{
}

void defaults_reset()
{

    settings_setint("initialized", 1);

    settings_setint("sceneindex", 0);
    settings_setint("hudvisible", 1);

    settings_setfloat("musicvolume", .5);
    settings_setfloat("soundvolume", .5);
    settings_setfloat("zoomratio", .5);
    settings_setfloat("alpharatio", .5);

    settings_setint("effects_level", 2);
}

void defaults_save()
{

    settings_setint("sceneindex", defaults.sceneindex);
    settings_setint("hudvisible", defaults.hudvisible);

    settings_setfloat("musicvolume", defaults.musicvolume);
    settings_setfloat("soundvolume", defaults.soundvolume);
    settings_setfloat("zoomratio", defaults.zoomratio);
    settings_setfloat("alpharatio", defaults.alpharatio);

    settings_setint("effects_level", defaults.effects_level);
}

#endif
