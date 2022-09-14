

    #ifndef menuelement_h
    #define menuelement_h


    #include "element.h"


    #define kMenuItemTypeGap    0
    #define kMenuItemTypeLabel  1
    #define kMenuItemTypeButton 2
    #define kMenuItemTypeToggle 3
    #define kMenuItemTypeSlider 4
    #define kMenuItemTypeTitle  5


    typedef struct _menuitem_t
    {
        char    	type;
        char*       label;
        char*       action;
        uint32_t    color;
        
    } menuitem_t;

    typedef struct _menudata_t
    {
        float innerheight;
        
    } menudata_t;

    element_t*  menuelement_alloc(char *name,float x,float y,float width,float height,float scale,textstyle_t text,font_t *font,uint32_t count,menuitem_t *items);
    void        menuelement_input(element_t *element,input_t *input);


    #endif
