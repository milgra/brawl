
	#ifndef _audio_header
	#define _audio_header

	void audio_init( char* path );
	void audio_free( void );

	#endif

	#if __INCLUDE_LEVEL__ == 0

	#include "defaults.c"
    #include "SDL_mixer.h"
	#include "mtbus.c"
	#include "mtcstr.c"


	Mix_Chunk* sounds[ 9 ];
	Mix_Music* music;


	void audio_onmessage( const char* name , void* data );


	void audio_free( )
	{
	
		Mix_FreeMusic( music );
		
		for ( int index = 0 ;
				  index < 9 ;
				  index++ )
		{
			Mix_FreeChunk( sounds[ index ] );
		}
		
	}


	void audio_init( char* path )
	{
	
		mtbus_subscribe( "SND" ,
						 audio_onmessage );
		
		char* msc = mtcstr_fromformat( "%s/theme.wav" ,
									   path ,
									   NULL );
		
		music = Mix_LoadMUS( msc );

		Mix_VolumeMusic( ( int )( defaults.musicvolume * 128.0 ) );

		mtmem_release( msc );

        /* cache effects */

		char* snd1 = mtcstr_fromformat( "%s/argh0.wav" , path , NULL );
		char* snd2 = mtcstr_fromformat( "%s/argh1.wav" , path , NULL );
		char* snd3 = mtcstr_fromformat( "%s/argh2.wav" , path , NULL );
		char* snd4 = mtcstr_fromformat( "%s/punch0.wav" , path , NULL );
		char* snd5 = mtcstr_fromformat( "%s/punch1.wav" , path , NULL );
		char* snd6 = mtcstr_fromformat( "%s/punch2.wav" , path , NULL );
		char* snd7 = mtcstr_fromformat( "%s/death0.wav" , path , NULL );
		char* snd8 = mtcstr_fromformat( "%s/death1.wav" , path , NULL );
		char* snd9 = mtcstr_fromformat( "%s/shot.wav" , path , NULL );
		
        Mix_Chunk* tmpsounds[9] =
        {
			Mix_LoadWAV( snd1 ) ,
			Mix_LoadWAV( snd2 ) ,
			Mix_LoadWAV( snd3 ) ,
			Mix_LoadWAV( snd4 ) ,
			Mix_LoadWAV( snd5 ) ,
			Mix_LoadWAV( snd6 ) ,
			Mix_LoadWAV( snd7 ) ,
			Mix_LoadWAV( snd8 ) ,
			Mix_LoadWAV( snd9 )
        };
		
        memcpy( sounds , tmpsounds , sizeof( tmpsounds ) );
		
        mtmem_releaseeach( snd1, snd2, snd3, snd4, snd5, snd6, snd7, snd8, snd9, NULL );
        
	}

	void audio_onmessage( const char* name , void* data )
	{
	
		if ( strcmp( name , "PLAYSND" ) == 0 )
		{
			char* type = data;
			
			int pick = 0;
			
			if ( strcmp( type , "argh" ) == 0 )
			{
				pick = rand( ) % 3;
			}
			else if ( strcmp( type , "hit" ) == 0 )
			{
				pick = 3 + rand( ) % 3;
			}
			else if ( strcmp( type , "death" ) == 0 )
			{
				pick = 6 + rand( ) % 2;
			}
			else if ( strcmp( type , "shoot" ) == 0 )
			{
				pick = 8;
			}

			Mix_Chunk* picksound = sounds[ pick ];
			Mix_VolumeChunk( picksound , ( int )( defaults.soundvolume * 128.0 ) );
			Mix_PlayChannel( -1 , picksound , 0 );
			
		}
		else if ( strcmp( name , "PLAYMUSIC" ) == 0 )
		{

			Mix_PlayMusic( music , -1 );

		}
		else if ( strcmp( name , "MUSICVOLUME" ) == 0 )
		{

			defaults.musicvolume = * ( float* ) data;
			defaults_save( );
		
			Mix_VolumeMusic( ( int )( defaults.musicvolume * 128.0 ) );

		}
		else if ( strcmp( name , "SOUNDVOLUME" ) == 0 )
		{

			defaults.soundvolume = * ( float* ) data;
			defaults_save( );

		}
		else if ( strcmp( name , "FOREGROUND" ) == 0 )
		{
			Mix_ResumeMusic();
		}
		else if ( strcmp( name , "BACKGROUND" ) == 0 )
		{
			Mix_PauseMusic();
		}

	}

	#endif
