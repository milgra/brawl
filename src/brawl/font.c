
#ifndef font_h
#define font_h

#include "stb_truetype.h"
#include "text.c"
#include "zc_bm_rgba.c"
#include "zc_string.c"
#include "zc_vector.c"
#include <stdint.h>

typedef struct _font_t font_t;
struct _font_t
{
    stbtt_fontinfo info;
    unsigned char* buffer;
    int            gap;
    int            ascent;
    int            descent;
};

font_t*    font_alloc(char* the_font_path);
void       font_dealloc(void* the_font);
bm_rgba_t* font_render_text(
    int             width,
    int             height,
    str_t*          string,
    font_t*         the_font,
    textstyle_t     text,
    glyphmetrics_t* glyphmetrics,
    vec_t*          selections);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "floatbuffer.c"
#include "zc_cstring.c"
#include "zc_draw.c"
#include "zc_memory.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#define STB_TRUETYPE_IMPLEMENTATION // force following include to generate implementation
#include "../ext/stb_truetype.h"

/* alloc font */

font_t* font_alloc(char* font_path)
{
    struct stat filestat;
    char        success = stat(font_path, &filestat);

    if (success == 0 && filestat.st_size > 0)
    {
	FILE* font_file = fopen(font_path, "rb");

	if (font_file != NULL)
	{
	    // read file

	    font_t* font = CAL(sizeof(font_t), font_dealloc, NULL);
	    font->buffer = CAL((size_t) filestat.st_size, NULL, NULL);

	    fread(font->buffer, (size_t) filestat.st_size, 1, font_file);

	    // create font

	    char success = stbtt_InitFont(&(font->info), font->buffer, stbtt_GetFontOffsetForIndex(font->buffer, 0));
	    if (success == 1)
	    {
		// get font_ascent font_descent and line font_gap

		stbtt_GetFontVMetrics(&(font->info), &(font->ascent), &(font->descent), &(font->gap));

		return font;
	    }
	    else
	    {
		REL(font->buffer);
		REL(font);
	    }

	    fclose(font_file);
	}
    }

    return NULL;
}

/* dealloc font */

void font_dealloc(void* pointer)
{
    font_t* the_font = pointer;
    REL(the_font->buffer);
}

/* render text TODO !!! CLEANUP, REFACTOR */

bm_rgba_t* font_render_text(int width, int height, str_t* string, font_t* the_font, textstyle_t text, glyphmetrics_t* glyphmetrics, vec_t* selections)
{
    /* get font metrics */

    float scale = stbtt_ScaleForPixelHeight(&(the_font->info), text.textsize);
    float desc  = the_font->descent * scale;
    float asc   = the_font->ascent * scale;
    float gap   = the_font->gap * scale;

    /* return empty bitmap in case of no text */

    v2_t            dimensions = v2_init(width, asc + -desc);
    glyphmetrics_t* metrics    = glyphmetrics;

    if (glyphmetrics == NULL)
    {
	metrics = CAL(sizeof(glyphmetrics_t) * (string == NULL ? 2 : string->length + 2), NULL, NULL);
    }

    if (string == NULL || string->length == 0)
    {
	/* align empty text to init metrics */

	text_align(metrics, text, &dimensions, string == NULL ? 0 : string->length, asc, desc, gap);

	int xoff = 0;
	int yoff = 0;
	if (dimensions.x < width) xoff = (width - dimensions.x) / 2;
	if (dimensions.y < height) yoff = (height - dimensions.y) / 2;
	if (text.autosize == 1 && text.multiline == 1) yoff = 0;

	metrics[0].x += xoff;
	metrics[0].y += yoff;

	/* create empty bitmap */

	bm_rgba_t* result = bm_rgba_new(dimensions.x, dimensions.y);
	gfx_rect(result, 0, 0, result->w, result->h, text.backcolor, 0);

	if (glyphmetrics == NULL) REL(metrics);
	return result;
    }

    /* bitmap array with maximum possible characters */

    bm_rgba_t* bitmaps[string->length];

    /* generate glyphs and get metrics */

    for (int index = 0; index < string->length; index++)
    {
	int gw, gh, gxo, gyo, kern;

	/* default font colorts */

	uint32_t backcolor = text.backcolor & 0xFFFFFF00;
	uint32_t textcolor = text.textcolor;
	uint8_t  selected  = 0;

	/* if given index is selected, use selection colors as font colors */

	if (selections != NULL && selections->length > 0)
	{
	    for (int si = 0; si < selections->length; si++)
	    {
		textselection_t* selection = selections->data[si];
		if (selection->startindex <= index && selection->endindex > index)
		{
		    // backcolor = selection->backcolor;
		    textcolor = selection->textcolor;
		    selected  = 1;
		}
	    }
	}

	/* generate raw font bitmap */

	unsigned char* rawmap = stbtt_GetCodepointBitmap(&(the_font->info), scale, scale, string->codepoints[index], &gw, &gh, &gxo, &gyo);

	/* get kerning */

	if (index > 0)
	{
	    kern = scale * stbtt_GetCodepointKernAdvance(&(the_font->info), string->codepoints[index - 1], string->codepoints[index]);
	}
	else kern = 0;

	/* space needs plus width */

	if (string->codepoints[index] == 32)
	{
	    int aw, db;
	    stbtt_GetCodepointHMetrics(&(the_font->info), string->codepoints[index], &aw, &db);
	    gw = (aw + db) * scale;
	}

	/* make carriage return and line feed invisible */

	if (string->codepoints[index] == 10 || string->codepoints[index] == 13) memset(rawmap, 0, gw * gh);

	/* generate and store final bitmap */

	bm_rgba_t* bitmap;
	if (gw > 0 && gh > 0)
	{
	    bitmap = gfx_bm_rgba_from_grayscale(gw, gh, backcolor & 0xFFFFFF00, textcolor, rawmap);
	    free(rawmap);
	}
	else
	{
	    bitmap = bm_rgba_new(1, 1);
	}

	bitmaps[index] = bitmap;

	/* store metrics at index + 1 because 0 is a 0 width char */

	metrics[index + 1] = glyphmetrics_init(string->codepoints[index], gw, gh, gxo, gyo, (float) kern * scale, selected);
    }

    /* align text */

    text_align(metrics, text, &dimensions, string->length, asc, desc, gap);

    /* draw all glyphs, create the base bitmap first */

    bm_rgba_t* result = bm_rgba_new(dimensions.x < width ? width : dimensions.x, dimensions.y < height ? height : dimensions.y);

    gfx_rect(result, 0, 0, result->w, result->h, text.backcolor, 0);

    int xoff = 0;
    int yoff = 0;

    /* center horizontally and vertically */

    if (dimensions.x < width) xoff = (width - dimensions.x) / 2;
    if (dimensions.y < height) yoff = (height - dimensions.y) / 2;

    metrics[0].x += xoff;
    metrics[0].y += yoff;

    int prevx = metrics[0].x;
    int prevy = metrics[0].y;
    int prevr = -1;

    for (int index = 0; index < string->length; index++)
    {
	bm_rgba_t*      bitmap = bitmaps[index];
	glyphmetrics_t* glyph  = &metrics[index + 1];

	glyph->x += xoff;
	glyph->y += yoff;

	if (glyph->selected == 1)
	{
	    if (glyph->row != prevr)
	    {
		prevx = glyph->x;
		prevy = glyph->y;
	    }
	    gfx_rect(result, prevx, prevy - asc, glyph->x + glyph->width, glyph->y - desc, 0xBBBBBBFF, 0);
	}

	if (bitmap->data != NULL) gfx_blend_bitmap(result, bitmap, glyph->x, glyph->y + glyph->yoff);

	prevx = glyph->x + glyph->width;
	prevy = glyph->y;
	prevr = glyph->row;

	REL(bitmap);
    }

    if (glyphmetrics == NULL) REL(metrics);

    return result;
}

#endif
