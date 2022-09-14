
	#ifndef actor_modifier_jump_points_h
	#define actor_modifier_jump_points_h

    #include "actor.c"

    void actor_modifier_jump_points_new_hands( actor_modifier_t* modifier , actor_modifier_args_t* args );
    void actor_modifier_jump_points_new_hip( actor_modifier_t* modifier , actor_modifier_args_t* args );
    void actor_modifier_jump_points_new_feet( actor_modifier_t* modifier , actor_modifier_args_t* args );

	#endif /* actor_modifier_jump_points_h */

	#if __INCLUDE_LEVEL__ == 0

    #include "actor_modifier_jump_points.c"
    #include "actor_modifier_jump.c"


    /* new feet points for jump animation */

    void actor_modifier_jump_points_new_feet( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
        actor_t* actor = args->actor;
        actor_modifier_jump_t* data = modifier->data;

        actor->points.ankle_a = actor->points.base_a;
        actor->points.ankle_b = actor->points.base_b;
        
        /* init kick state if needed */

        if ( args->control_state.kick_pressed == 1 && actor->control_state.kick_pressed == 0 && data->kick.speed == 0.0 && actor->power >= actor->metrics.hitpower )
		{
			data->kick = movement_alloc( 0.0 , 7.0 , 0.0 , 80.0 , 1 );
			actor->power -= actor->metrics.hitpower;
		}

		actor->points.ankle_a.x += data->kick.size * actor->facing;
		actor->points.ankle_a.y += data->kick.size;
        
        if ( data->kick.speed != 0.0 )
		{
			movement_step( &data->kick , args->delta );
			if ( data->kick.size == data->kick.maximum )
			{
				actor_modifier_jump_sendkickaction( modifier , args );
			}
		}
		
    }

    /* new hip points for animation */

    void actor_modifier_jump_points_new_hip( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
        actor_t* actor = args->actor;
        actor_modifier_jump_t* data = modifier->data;

        /* calculate hip base point based on bases */
        
        v2_t base_a = actor->points.base_a;
        v2_t base_b = actor->points.base_b;
        v2_t base_c = v2_midpoints( base_a , base_b );
        
        /* default position over base center */
        
        base_c.y += actor->metrics.leglength * 0.85 + fabsf( base_b.x - base_a.x ) / 10.0;
        base_c.x += actor->facing * 10.0;

        /* stand up slowly if squatting */
        
        if ( data->squatsize < 20.0 ) data->squatsize += .4;
        
        /* add hip modifier for jumping */
        
        base_c.y += data->hipmod;
        
        /* add squat modifier */
        
        base_c.y -= data->squatsize;
        
        /* set hip */

        actor->points.hip = base_c;

        v2_t ankle_a = actor->points.ankle_a;
        v2_t ankle_b = actor->points.ankle_b;

        actor->points.knee_b = v2_triangle_with_bases( actor->points.hip, ankle_b, actor->metrics.leglength / 2.0, actor->facing );
        actor->points.knee_a = v2_triangle_with_bases( actor->points.hip, ankle_a, actor->metrics.leglength / 2.0, actor->facing );

        float neckx = actor->points.hip.x + actor->facing * 1.0;
        float necky = actor->points.hip.y + actor->metrics.bodylength;
    
        actor->points.neck = v2_init( neckx , necky );
        actor->points.head = v2_init( neckx , necky + actor->metrics.headlength );
        
    }

    /* new hand points for jump animation */

    void actor_modifier_jump_points_new_hands( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
        actor_t* actor = args->actor;
        actor_modifier_jump_t* data = modifier->data;

        /* init punch if needed */
        
        if ( args->control_state.punch_pressed == 1 &&
             args->control_state.block_pressed == 0 &&
             actor->control_state.punch_pressed == 0 &&
             actor->gothit == 0 )
        {
            if ( actor->power >= actor->metrics.hitpower )
            {
                if ( data->punch_a.speed == 0.0 )
                {
					data->punch_a = movement_alloc( 0.0 , actor->metrics.punchspeed , 0.0 , actor->metrics.armlength , 1 );
                    actor->power -= actor->metrics.hitpower;
                }
                else if ( actor->power > actor->metrics.hitpower && data->punch_b.speed == 0.0 )
                {
					data->punch_b = movement_alloc( 0.0 , actor->metrics.punchspeed , 0.0 , actor->metrics.armlength , 1 );
                    actor->power -= actor->metrics.hitpower;
                }
            }
        }

        if ( data->punch_a.speed != 0.0 )
        {
        	movement_step( &data->punch_a , args->delta );
        	if ( data->punch_a.size == data->punch_a.maximum )
        	{
 				actor_modifier_jump_sendpunchaction( modifier , args , v2_sub( actor->points.hand_a , actor->points.neck ) );
        	}
		}
        if ( data->punch_b.speed != 0.0 )
        {
        	movement_step( &data->punch_b , args->delta );
        	if ( data->punch_b.size == data->punch_b.maximum )
        	{
				actor_modifier_jump_sendpunchaction( modifier , args , v2_sub( actor->points.hand_b , actor->points.neck ) );
        	}
		}

        v2_t neck = actor->points.neck;
    
        float hand_ax = actor->points.neck.x + actor->facing * ( actor->metrics.armlength * 0.3 + ( actor->points.base_b.x - actor->points.base_a.x ) / 8.0 );
        float hand_ay = actor->points.neck.y - actor->metrics.armlength * 0.1;
        float hand_bx = actor->points.neck.x + actor->facing * ( actor->metrics.armlength * 0.4 + ( actor->points.base_b.x - actor->points.base_a.x ) / 8.0 );
        float hand_by = actor->points.neck.y - actor->metrics.armlength * 0.1;

		hand_ax += (float)actor->facing * data->punch_a.size;
		hand_bx += (float)actor->facing * data->punch_b.size;

        actor->points.hand_a = v2_init( hand_ax , hand_ay );
        actor->points.elbow_a = v2_triangle_with_bases( neck , actor->points.hand_a, actor->metrics.armlength * .5, -1 * actor->facing );
        actor->points.hand_b = v2_init( hand_bx , hand_by );
        actor->points.elbow_b = v2_triangle_with_bases( neck , actor->points.hand_b, actor->metrics.armlength * .5, -1 * actor->facing );

    }

	#endif
