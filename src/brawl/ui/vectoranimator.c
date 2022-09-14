

    #include "vectoranimator.h"

    /* init animator */

    void vectoranimator_init( vectoranimator_t* animator , v2_t startvalue , v2_t endvalue , uint32_t starttime , uint32_t endtime , char type )
    {
        animator->type = type;
        animator->running = 0;
    
        animator->startvalue = startvalue;
        animator->endvalue = endvalue;
        animator->actual = startvalue;
        animator->delta = v2_sub( endvalue , startvalue );
        
        animator->starttime = starttime;
        animator->endtime = endtime;
        animator->duration = endtime - starttime;
    }

    /* stops animator */

    void vectoranimator_stop( vectoranimator_t* animator )
    {
        animator->running = 0;
        animator->starttime = 0;
        animator->endtime = 0;
    }

    /* iterates animator */

    v2_t vectoranimator_step( vectoranimator_t* animator , uint32_t time )
    {
        if ( time >= animator->starttime && time <= animator->endtime )
        {
            animator->running = 1;

            float ( *tween )( float start , float delta , float time , float duration ) = NULL;
            
            switch ( animator->type )
            {
                case kAnimationTypeLinear : tween = float_tween_linear; break;
                
                case kAnimationTypeEaseInQuadratic : tween = float_tween_ease_in_quadratic; break;
                case kAnimationTypeEaseOutQuadratic : tween = float_tween_ease_out_quadratic; break;
                case kAnimationTypeEaseInEaseOutQuadratic : tween = float_tween_ease_in_ease_out_quadratic; break;
                
                case kAnimationTypeEaseInCubic : tween = float_tween_ease_in_cubic; break;
                case kAnimationTypeEaseOutCubic : tween = float_tween_ease_out_cubic; break;
                case kAnimationTypeEaseInEaseOutCubic : tween = float_tween_ease_in_ease_out_cubic; break;
                
                case kAnimationTypeEaseInQuartic : tween = float_tween_ease_in_quartic; break;
                case kAnimationTypeEaseOutQuartic : tween = float_tween_ease_out_quartic; break;
                case kAnimationTypeEaseInEaseOutQuartic : tween = float_tween_ease_in_ease_out_quartic; break;
                
                case kAnimationTypeEaseInQuintic : tween = float_tween_ease_in_quintic; break;
                case kAnimationTypeEaseOutQuintic : tween = float_tween_ease_out_quintic; break;
                case kAnimationTypeEaseInEaseOutQuintic : tween = float_tween_ease_in_ease_out_quintic; break;
                
                case kAnimationTypeEaseInSine : tween = float_tween_ease_in_sine; break;
                case kAnimationTypeEaseOutSine : tween = float_tween_ease_out_sine; break;
                case kAnimationTypeEaseInEaseOutSine : tween = float_tween_ease_in_ease_out_sine; break;
                
                case kAnimationTypeEaseInExp : tween = float_tween_ease_in_exp; break;
                case kAnimationTypeEaseOutExp : tween = float_tween_ease_out_exp; break;
                case kAnimationTypeEaseInEaseOutExp : tween = float_tween_ease_in_ease_out_exp; break;
                
                case kAnimationTypeEaseInCirc : tween = float_tween_ease_in_circ; break;
                case kAnimationTypeEaseOutCirc : tween = float_tween_ease_out_circ; break;
                case kAnimationTypeEaseInEaseOutCirc : tween = float_tween_ease_in_ease_out_circ; break;
            }

            animator->actual.x = (*tween)( animator->startvalue.x , animator->delta.x , time - animator->starttime , animator->duration );
            animator->actual.y = (*tween)( animator->startvalue.y , animator->delta.y , time - animator->starttime , animator->duration );
        
        }
        else
        {
            animator->actual = v2_add( animator->startvalue , animator->delta );
            animator->running = 0;
        }
        
        return animator->actual;
    }
