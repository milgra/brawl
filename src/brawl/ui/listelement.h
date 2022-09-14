

    #ifndef listelement_h
    #define listelement_h


    #include "list.h"
    #include "element.h"
    #include "textelement.h"
    #include "input.h"

    typedef struct _listdata_t
    {
        list_t* list;                   /* abstract list loginc */

        mtvec_t* items;                 /* list items */
        mtvec_t* headqueue;             /* elements waiting for to be added to head */
        mtvec_t* tailqueue;             /* elements waiting for to be added to tail */

        element_t* scrollbar;           /* scrollbar */        

        char bounce;                    /* list should bounce */
        char firepull;                  /* pull event should be fired */
        char buffersize;                /* maximum element number in head/tail buffers */
        char newatbottom;               /* new elements should be at bottom */
        
        char dragging;                  /* indicates that list is being dragged */
        char scrolling;                 /* indicates that list is being scrolled */
        char rendering;                 /* indicates if list elements are being renderer in a different thread */

        float zoom;                     /* zoom factor */
        float momentum;
        float autospeed;
        float lasttouch;
        
        v2_t translation;          /* translation of elements */
        v2_t scrolltranslation;    /* translation of scrollbar */

    } listdata_t;


    element_t *listelement_alloc(char *name,float x,float y,float width,float height,uint32_t color,char enablex,char enabley,char newatbottom,uint32_t buffersize,float scrollerwidth);

    char listelement_reset(element_t *element);

    char listelement_isempty(element_t *element);

    char listelement_needstailitems(element_t *element);

    char listelement_needsheaditems(element_t *element);

    element_t *listelement_tailitem(element_t *element);

    element_t *listelement_headitem(element_t *element);

    char listelement_queuetailitem(element_t *element,element_t *item);

    char listelement_queueheaditem(element_t *element,element_t *item);

    void listelement_updateindexes(element_t *element,int headindex,int maxindex);

    void listelement_autoscroll(element_t *element,input_t *input);

    void listelement_update(element_t *element,input_t *input);



    #endif
