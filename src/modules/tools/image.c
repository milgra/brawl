
#ifndef image_h
#define image_h

#include "mtbmp.c"
#include "zc_memory.c"

mtbmp_t* image_bmp_from_png(char* path);

#endif

#if __INCLUDE_LEVEL__ == 0

#define STB_IMAGE_IMPLEMENTATION // force following include to generate implementation
#include "../ext/stb_image.h"

// creates bitmap from a png

mtbmp_t* image_bmp_from_png(char* path)
{
    mtbmp_t* bitmap = CAL(sizeof(mtbmp_t), mtbmp_dealloc, NULL);

    if (path != NULL)
    {
	FILE* file = fopen(path, "r");

	if (file == NULL) printf("ERROR file does not exists %s", path);
	else
	{

	    fclose(file);

	    int components;

	    unsigned char* bytes = stbi_load(path, &(bitmap->width), &(bitmap->height), &components, 4);
	    bitmap->bytes_length = 4 * bitmap->width * bitmap->height;
	    bitmap->bytes        = CAL(bitmap->bytes_length, NULL, NULL);
	    memcpy(bitmap->bytes, bytes, bitmap->bytes_length);
	    stbi_image_free(bytes);
	}
    }

    return bitmap;
}

#endif
