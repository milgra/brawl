
#ifndef image_h
#define image_h

#include "zc_bm_rgba.c"
#include "zc_memory.c"

bm_rgba_t* image_bmp_from_png(char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#define STB_IMAGE_IMPLEMENTATION // force following include to generate implementation
#include "../ext/stb_image.h"

// creates bitmap from a png

bm_rgba_t* image_bmp_from_png(char* path)
{
    bm_rgba_t* bitmap = NULL;

    if (path != NULL)
    {
	FILE* file = fopen(path, "r");

	if (file == NULL) printf("ERROR file does not exists %s", path);
	else
	{
	    fclose(file);

	    int components;
	    int width;
	    int height;

	    unsigned char* bytes = stbi_load(path, &width, &height, &components, 4);
	    bitmap               = bm_rgba_new(width, height);
	    memcpy(bitmap->data, bytes, bitmap->size);
	    stbi_image_free(bytes);
	}
    }

    return bitmap;
}

#endif
