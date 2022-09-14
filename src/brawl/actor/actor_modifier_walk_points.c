
	#ifndef actor_modifier_walk_points_h
	#define actor_modifier_walk_points_h

    #include "actor.c"

    void actor_modifier_walk_points_new_hands( actor_modifier_t* modifier , actor_modifier_args_t* args );
    void actor_modifier_walk_points_new_feet( actor_modifier_t* modifier , actor_modifier_args_t* args );
    void actor_modifier_walk_points_new_hip( actor_modifier_t* modifier , actor_modifier_args_t* args );

	#endif /* actor_modifier_walk_points_h */

	#if __INCLUDE_LEVEL__ == 0

    #include "actor_modifier_walk_points.c"
    #include "actor_modifier_walk.c"

	/* hand positions */

    void actor_modifier_walk_points_new_hands( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
        actor_t* actor = args->actor;
        actor_modifier_walk_t* data = modifier->data;

        /* init punch if needed */
        
        if ( args->control_state.punch_pressed == 1 &&
             args->control_state.block_pressed == 0 &&
             actor->control_state.punch_pressed == 0 &&
             actor->gothit == 0 )
        {
            if ( actor->power >= actor->metrics.hitpower )
            {
                if ( data->puncha.speed == 0.0 )
                {
					data->puncha = movement_alloc( 0.0 , actor->metrics.punchspeed , 0.0 , actor->metrics.armlength , 1 );
                    actor->power -= actor->metrics.hitpower;
                }
                else if ( actor->power >= actor->metrics.hitpower && data->punchb.speed == 0.0 )
                {
					data->punchb = movement_alloc( 0.0 , actor->metrics.punchspeed , 0.0 , actor->metrics.armlength , 1 );
                    actor->power -= actor->metrics.hitpower;
                }
            }
        }

        /* init or stop blocking */
		
        if ( args->control_state.kick_pressed == 0 && actor->control_state.kick_pressed == 1 )
        {
        	actor->control_state.block_pressed = 0;
		}
        if ( args->control_state.punch_pressed == 0 && actor->control_state.punch_pressed == 1 )
        {
        	actor->control_state.block_pressed = 0;
		}

        if ( args->control_state.block_pressed == 1 &&
        	 args->control_state.punch_pressed == 0 &&
        	 args->control_state.kick_pressed == 0 &&
        	 actor->control_state.block_pressed == 0 )
        {
			data->block = movement_alloc( 0.0 , 5.0 , 0.0 , 50.0 , 0 );
		}

        if ( args->control_state.block_pressed == 0 && data->block.speed > 0 )
        {
			data->block.speed *= -1;
		}
		
		/* calculate points */
        
        float neckx = actor->points.hip.x + actor->facing * fabs( actor->points.base_b.x - actor->points.base_a.x ) / 8.0;
        float necky = actor->points.hip.y + actor->metrics.bodylength;
        
        /* bring neck forward in case of squat */
        
        neckx += (-actor->facing * data->squatsize) / 2.0;
        necky += data->squatsize / 4.0;
                
        /* calculate hand positions */

        float handax = actor->facing * ( actor->metrics.armlength * .4 + ( actor->points.base_b.x - actor->points.base_a.x ) / 8.0 );
        float handbx = actor->facing * ( actor->metrics.armlength * .4 - ( actor->points.base_b.x - actor->points.base_a.x ) / 8.0 );
        
        float handay = -actor->metrics.armlength * 0.1;
        float handby = -actor->metrics.armlength * 0.14;
        
        if ( data->puncha.speed != 0.0 )
        {
        	movement_step( &data->puncha , args->delta );
			if ( data->puncha.size == data->puncha.maximum ) actor_modifier_walk_sendpunchaction( modifier , args , v2_sub( actor->points.hand_a , actor->points.neck ) );
		}
		
        if ( data->punchb.speed != 0.0 )
        {
        	movement_step( &data->punchb , args->delta );
			if ( data->punchb.size == data->punchb.maximum ) actor_modifier_walk_sendpunchaction( modifier , args , v2_sub( actor->points.hand_b , actor->points.neck ) );
		}
		
        if ( args->control_state.shoot_pressed == 1 )
        {
            handax += (float)actor->facing * actor->metrics.armlength * 0.6;
            handay += 10.0;
            handbx += (float)actor->facing * actor->metrics.armlength * 0.4;
            handby += 10.0;
        }
        else
        {
            handax += (float)actor->facing * data->puncha.size;
            handbx += (float)actor->facing * data->punchb.size;
        }
		
		movement_step( &data->block , args->delta );

		handax += actor->facing * -data->block.size / 10.0;
		handay += data->block.size / 2.0;
		handbx += actor->facing * -data->block.size / 8.0;
		handby += data->block.size / 1.5;
		
		if ( data->puncha.speed != 0.0 || data->punchb.speed != 0.0 )
		{		
			neckx += handax / 10.0 + handbx / 10.0;
			necky += handax / 10.0 - handbx / 10.0;
		}
        
        /* neck and head position */

        actor->points.neck = v2_init( neckx , necky );
        actor->points.head = v2_init( neckx , necky + actor->metrics.headlength );
        
        /* breathing */

        data->breathangle += 0.05 * args->delta;
        
        if ( data->breathangle > 2 * M_PI ) data->breathangle = 0.0;
		
        if ( args->control_state.shoot_pressed == 0 )
        {
            handax += sinf( data->breathangle ) * 5.0;
            handay += cosf( data->breathangle ) * 5.0;
            handbx -= sinf( data->breathangle ) * 5.0;
            handby -= cosf( data->breathangle ) * 5.0;
        }
        
        /* hand and elbow positions */
        
        actor->points.hand_a = v2_init( actor->points.neck.x + handax , actor->points.neck.y + handay );
        actor->points.hand_b = v2_init( actor->points.neck.x + handbx , actor->points.neck.y + handby );
        
        actor->points.elbow_b = v2_triangle_with_bases( actor->points.neck , actor->points.hand_b, actor->metrics.armlength * .5, -1 * actor->facing );
        actor->points.elbow_a = v2_triangle_with_bases( actor->points.neck , actor->points.hand_a, actor->metrics.armlength * .5, -1 * actor->facing );
    }

    /* new feet positions */
    
    void actor_modifier_walk_points_new_feet( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
        actor_t* actor = args->actor;
        actor_modifier_walk_t* data = modifier->data;

        actor->points.ankle_a = actor->points.base_a;
        actor->points.ankle_b = actor->points.base_b;

        actor->points.ankle_a.y -= 2.0;
        actor->points.ankle_b.y -= 2.0;
        
        /* set only if moving */
        
        if ( fabs( actor->speed.x ) > 0.0 )
        {
            float act_dy = 0.0;
            float pas_dy = 0.0;
            float run_act_dy = 0.0;
            float run_pas_dy = 0.0;
            float walk_act_dy = 0.0;
            float walk_pas_dy = 0.0;

            if ( fabs(actor->speed.x) > 0.1 && data->steplength > 1.0 )
            {
                float step_ratio = 1.0;
                float step_delta = v2_length( v2_sub( data->sight.final_point , *(data->activebase) ) );
                
                /* walking */
                
                if ( step_delta > data->steplength / 2.0 )
                {
                    step_ratio = ( data->steplength - step_delta ) / data->steplength;
                }
                else step_ratio = step_delta / data->steplength;

                walk_act_dy = fabs(actor->speed.x * 6.0) * step_ratio;
                walk_pas_dy = 0.0;

                /* running */
                
                if ( step_delta < data->steplength / 3.0 )
                {
                    step_ratio = ( data->steplength / 3.0 - step_delta ) / ( data->steplength / 3.0 );
                    run_pas_dy = actor->metrics.leglength * .5 * step_ratio;
                }

                step_ratio = step_delta / data->steplength;
                run_act_dy = actor->metrics.leglength * .5 * step_ratio;
            }
            
            /* merge running and walking state when slowing/speeding from/to running */
            
            if ( fabs(actor->speed.x) > actor->metrics.walkspeed )
            {        
                float dif_speed = actor->metrics.runspeed - fabs(actor->speed.x);
                float walk_ratio = dif_speed / ( actor->metrics.runspeed - actor->metrics.walkspeed );
                float run_ratio = 1.0 - walk_ratio;

                act_dy = walk_ratio * walk_act_dy + run_ratio * run_act_dy;
                pas_dy = walk_ratio * walk_pas_dy + run_ratio * run_pas_dy;
            }
            else
            {
                act_dy = walk_act_dy;
                pas_dy = walk_pas_dy;
            }
            
            /* set final positions */
            
            if ( data->activebase == &actor->points.base_a )
            {
                actor->points.ankle_a.y += act_dy + 1.0;
                actor->points.ankle_b.y += pas_dy + 1.0;
            }
            else
            {
                actor->points.ankle_a.y += pas_dy + 1.0;
                actor->points.ankle_b.y += act_dy + 1.0;
            }
        }
    }

    /* new hip position */

    void actor_modifier_walk_points_new_hip( actor_modifier_t* modifier , actor_modifier_args_t* args )
    {
		
        actor_t* actor = args->actor;
        actor_modifier_walk_t* data = modifier->data;
                
        /* start squat if down */
        
        if ( actor->control_state.squat_pressed == 0 && args->control_state.squat_pressed == 1 )
        {
        	actor->speed.y = -10.0;
		}

        if ( actor->speed.y < -1.0 )
        {
            /* squat down */
            
            data->squatsize += actor->speed.y;
            actor->speed.y *= .8;

            if ( data->squatsize < -actor->metrics.leglength / 2.0 )
            {
            	data->squatsize = -actor->metrics.leglength / 2.0;
			}
			
        }
        else if ( actor->speed.y > -1.0 && actor->speed.y <  0.0 )
        {
            /* squat down end */
            
            if ( args->control_state.squat_pressed == 0 ) actor->speed.y = 2.0;
			
        }
        else if ( actor->speed.y > 0.9 )
        {
            /* squat up */
            
            data->squatsize += actor->speed.y * args->delta;

            if ( data->squatsize > 0.0 )
            {
                actor->speed.y = 0.05;
                data->squatsize = 0.0;
            }
        }

        /* calculate point */
        
        v2_t base_a = actor->points.base_a;
        v2_t base_b = actor->points.base_b;
        v2_t base_c = v2_sub( base_b , base_a );
        
        base_c = v2_scale( base_c , 0.5 );
        base_c = v2_add( base_a , base_c );
        
        /* default position over base center */
        
        base_c.y += actor->metrics.leglength * .85 + fabsf( base_b.x - base_a.x ) / 10.0;
        base_c.y += data->squatsize;

        base_c.y += sinf( data->breathangle ) * 2.0;
        
        /* higher hip position when standing */
        
        if ( fabsf( actor->speed.x ) < 3.0 )
        {
        	float dy = ( 3.0 - fabsf( actor->speed.x ) ) * 2.0 - fabsf( base_b.x - base_a.x ) / 5.0;
        	base_c.y += dy;
		}
        
        if ( args->control_state.right_pressed == 1 ||
             args->control_state.left_pressed  == 1 )
        {
            if ( actor->state == kActorStateWalk ) base_c.x += actor->facing * 2.0;
            else base_c.x += actor->facing * 10.0;
        }
        
        /* hip */

        actor->points.hip = base_c;

        v2_t ankle_a = actor->points.ankle_a;
        v2_t ankle_b = actor->points.ankle_b;
        
        /* knees */
        
        actor->points.knee_b = v2_triangle_with_bases( actor->points.hip , ankle_b, actor->metrics.leglength / 1.95, actor->facing );
        actor->points.knee_a = v2_triangle_with_bases( actor->points.hip , ankle_a, actor->metrics.leglength / 1.95, actor->facing );
    }

	#endif
