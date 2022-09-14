
#ifndef json_h
#define json_h

#include "jsmn.c"
#include "zc_map.c"

map_t* json_parse(char* string);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_cstring.c"
#include "zc_memory.c"
#include "zc_string.c"
#include <string.h>

/* parses c string into an mtmap */

map_t* json_parse(char* string)
{
    map_t* result = MNEW();

    jsmn_parser parser;
    jsmntok_t   tokens[128]  = {0}; /* We expect no more than 128 tokens */
    map_t*      objects[128] = {0};
    objects[0]               = result;

    jsmn_init(&parser);

    int count = jsmn_parse(&parser, string, strlen(string), tokens, sizeof(tokens) / sizeof(tokens[0]));

    if (count < 1) return result;
    if (tokens[0].type != JSMN_OBJECT) return result;

    for (int index = 1; index < count; index++)
    {
	jsmntok_t tok = tokens[index];

	if (tok.type == JSMN_ARRAY)
	{
	    jsmntok_t parent        = tokens[tok.parent];
	    char*     key           = strndup(string + parent.start, parent.end - parent.start);
	    char*     value         = strndup(string + tok.start, tok.end - tok.start);
	    char*     managedstring = cstr_new_cstring(value);
	    map_t*    parentmap     = objects[parent.parent];
	    map_put(parentmap, key, managedstring);

	    free(key);
	    free(value);
	    REL(managedstring);
	}
	else if (tok.type == JSMN_OBJECT)
	{
	    jsmntok_t parent = tokens[tok.parent];
	    char*     key    = strndup(string + parent.start, parent.end - parent.start);

	    map_t* map       = MNEW();
	    map_t* parentmap = objects[parent.parent];
	    MPUT(parentmap, key, map);

	    objects[index] = map;

	    REL(map);
	    free(key);
	}
	else
	{
	    jsmntok_t parent = tokens[tok.parent];

	    if (parent.type != JSMN_OBJECT && parent.type != JSMN_ARRAY)
	    {
		char* key   = strndup(string + parent.start, parent.end - parent.start);
		char* value = strndup(string + tok.start, tok.end - tok.start);

		char* mngstring = cstr_new_cstring(value);

		map_t* parentmap = objects[parent.parent];
		MPUT(parentmap, key, mngstring);

		free(key);
		free(value);
		REL(mngstring);
	    }
	}
    }
    return result;
}

#endif
