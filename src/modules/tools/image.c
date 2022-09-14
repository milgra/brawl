
    #ifndef image_h
    #define image_h

    #include "../core/mtmem.c"
    #include "../core/mtbmp.c"

    mtbmp_t* image_bmp_from_png( char* path );

    #endif

    #if __INCLUDE_LEVEL__ == 0

    #define STB_IMAGE_IMPLEMENTATION  // force following include to generate implementation
    #include "../ext/stb_image.h"

    // creates bitmap from a png

    mtbmp_t* image_bmp_from_png( char* path )
    {
        mtbmp_t* bitmap = mtmem_calloc( sizeof( mtbmp_t ) , mtbmp_dealloc );
    
        if ( path != NULL )
        {
            FILE *file = fopen( path , "r" );
            
            if ( file == NULL ) printf( "ERROR file does not exists %s" , path );
            else
            {
            
                fclose( file );
        
                int components;

				unsigned char* bytes = stbi_load( path , &(bitmap->width) , &(bitmap->height) , &components , 4 );
                bitmap->bytes_length = 4 * bitmap->width * bitmap->height;
                bitmap->bytes = mtmem_calloc( bitmap->bytes_length , NULL );
				memcpy( bitmap->bytes , bytes , bitmap->bytes_length );
				stbi_image_free( bytes );
            }
        }
        
        return bitmap;
    }

    #endif
