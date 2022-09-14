

    #ifndef notifierelement_h
    #define notifierelement_h


    #include <stdio.h>
    #include "element.h"
    #include "vectoranimator.h"

    #define kNotifierStateAppear 0
    #define kNotifierStateIdle 1
    #define kNotifierStateDisappear 2

    typedef struct _notifierdata_t
    {
        int state;
        int timer;
        mtvec_t* messagequeue;
        vectoranimator_t animator;
        element_t* messageelement;
    
    } notifierdata_t;

    element_t* notifierelement_alloc(
        char* name ,
        float x ,
        float y ,
        float width ,
        float height ,
        textstyle_t style ,
        font_t* font );

    void notifierelement_queuemessage( element_t* element , mtstr_t* message , input_t* input );


    #endif
