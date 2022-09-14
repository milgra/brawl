

    #ifndef sliderelement_h
    #define sliderelement_h


    #include "element.h"

    typedef struct _sliderdata_t
    {
        char    editable;
        char    animated;
        
        float   ratio;
        float   actual;

    } sliderdata_t;

    element_t* sliderelement_alloc(
        char* name ,
        float x ,
        float y ,
        float width ,
        float height ,
        uint32_t frontcolor ,
        uint32_t backcolor ,
        char editable ,
        char animated );


    void    sliderelement_setslideaction( element_t* element , char* action );

    void    sliderelement_setratio( element_t* element , input_t* input , float ratio );

    float   sliderelement_getratio( element_t* element );


    #endif
