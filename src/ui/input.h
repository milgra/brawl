#ifndef KineticUI_input_h
#define KineticUI_input_h

#include "cmd.c"
#include "zc_string.c"

#define kInputTypeTimer 0

#define kInputTypeTouchDown 1
#define kInputTypeTouchDrag 2
#define kInputTypeTouchUp 3

#define kInputTypeScrollStart 4
#define kInputTypeScroll 5
#define kInputTypeScrollEnd 6

#define kInputTypeZoomStart 7
#define kInputTypeZoom 8
#define kInputTypeZoomEnd 9

#define kInputTypeKeyPress 10
#define kInputTypeKeyRelease 11

#define kInputTypeMouseMove 12

#define kInputTypeInit 13
#define kInputTypeUpdate 14
#define kInputTypeLayoutFix 15
#define kInputTypeLayoutUpdate 16
#define kInputTypeFocus 17
#define kInputTypeUnFocus 18
#define kInputTypeResize 19
#define kInputTypeRender 20
#define kInputTypeDomainError 21
#define kInputTypePushTokenArrived 22
#define kInputTypeVisibleFrameChanged 23
#define kInputTypeEnterForeground 24
#define kInputTypeEnterBackground 25
#define kInputTypeReturnSelection 26
#define kInputTypeVideoStarted 27
#define kInputTypeVideoStopped 28
#define kInputTypeGetSelected 29
#define kInputTypeLeftSwipe 30
#define kInputTypeRightSwipe 31
#define kInputTypeCopy 32
#define kInputTypePaste 33
#define kInputTypeTouchDownOutside 34
#define kInputTypeMediaLoaded 35
#define kInputTypeMediaSelected 36
#define kInputTypeMediaUploaded 37

#define kInputKeyTypeNormal 0
#define kInputKeyTypeReturn 1
#define kInputKeyTypeBackspace 2
#define kInputKeyTypeShift 3
#define kInputKeyTypeLeftArrow 4
#define kInputKeyTypeRightArrow 5
#define kInputKeyTypeUpArrow 6
#define kInputKeyTypeDownArrow 7
#define kInputKeyTypeSpace 8

typedef struct _input_t input_t;
struct _input_t
{
    /* primitive types */

    int type;
    int inta;
    int intb;

    char  key;
    char* name;
    char* stringa;
    char* stringb;
    char* stringc;

    float floata;
    float floatb;
    float floatc;

    /* element update related */

    uint32_t ticks;

    char upload;
    char render;

    char*       respath;
    cmdqueue_t* cmdqueue;

    /* cleanup? */

    float scale;
    char  chara;
    void* map;

    //		float value_x;
    //		float value_y;
    //      float ratio;
    //		mtstr_t* characters;
};

#endif
