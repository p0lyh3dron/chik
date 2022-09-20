/*
 *    platform.c    --    source for platform specific stuff
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    This file defines the platform specific stuff.
 *    Officially maintained is SDL2 and ALSA, if you want
 *    other platforms, implement these functions yourself
 *    and pull requests are welcome.
 */
#include "platform.h"

#if __unix__
#include <fcntl.h>
#endif /* __unix__  */

#if USE_ALSA
#include <alsa/asoundlib.h>
#endif /* USE_ALSA  */

#if USE_SDL
#include <SDL2/SDL.h>
#endif /* USE_SDL  */

#define PCM_DEVICE       "default"
#define PCM_CHANNELS     2
#define PCM_SAMPLE_RATE  44100
#define PCM_BUFFER_SIZE  8192
#define PCM_SAMPLE_WIDTH 16

#define DEFAULT_WIDTH  480
#define DEFAULT_HEIGHT 270
#define DEFAULT_TITLE  "Chik Application"

#define MAX_INPUT_TYPES 256
#define MAX_ALIAS_LENGTH 32

#define MAX_STDIN_READ 256

mempool_t *gpPlatformResources                                 = nullptr;
u32        gKeys[ MAX_INPUT_TYPES ]                            = { 0 };
s8         gpKeyAliases[ MAX_INPUT_TYPES ][ MAX_ALIAS_LENGTH ] = { { '\0' } };

#if USE_ALSA
snd_pcm_t *gpAudioDevice = nullptr;
#endif /* USE_ALSA  */

#if USE_SDL
SDL_Window   *gpWindow   = nullptr;
SDL_Renderer *gpRenderer = nullptr;
SDL_Texture  *gpTexture  = nullptr;

const s8 *gpKeyState                    = nullptr;
s8        gKeyMask[ SDL_NUM_SCANCODES ] = { 0 };
#endif /* USE_SDL  */

/*
 *    Initialize the audio device.
 */
u32 audio_init( void ) {
#if USE_ALSA
	u32 rate                     = PCM_SAMPLE_RATE;
	u32 channels                 = PCM_CHANNELS;
	snd_pcm_hw_params_t *pParams = nullptr;

	u32 ret;

	/*
	 *    Open the PCM device in playback mode
	 */
	if ( ret = snd_pcm_open( &gpAudioDevice, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0 ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Allocate parameters and set defaults. 
	 */
	snd_pcm_hw_params_alloca( &pParams );
	snd_pcm_hw_params_any( gpAudioDevice, pParams );

	/* 
	 *    Using interleaved, we store a sample in one channel followed by another in the other. 
	 */
	if ( ret = snd_pcm_hw_params_set_access( gpAudioDevice, pParams, SND_PCM_ACCESS_RW_INTERLEAVED ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't set interleaved mode. %s\n", snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Set the sample format to be 16 bits. 
	 */
	if ( ret = snd_pcm_hw_params_set_format( gpAudioDevice, pParams, SND_PCM_FORMAT_S16_LE ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't set format. %s\n", snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Set the number of channels to be 2. 
	 */	
	if ( ret = snd_pcm_hw_params_set_channels( gpAudioDevice, pParams, channels ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't set channels number. %s\n", snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Set the sample rate to be 44100 Hz. 
	 */
	if ( ret = snd_pcm_hw_params_set_rate_near( gpAudioDevice, pParams, &rate, 0 ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't set rate. %s\n", snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Apply the hardware parameters to the PCM device. 
	 */
	if ( ret = snd_pcm_hw_params( gpAudioDevice, pParams ) < 0 ) {
		log_warn( "u32 audio_init( void ): Can't set hardware parameters. %s\n", snd_strerror( ret ) );
		return 1;
	}

	/*
	 *    Display the current hardware parameters. 
	 */
	log_msg( "PCM name:       '%s'\n", snd_pcm_name( gpAudioDevice ) );
 	log_msg( "PCM state:       %s\n",  snd_pcm_state_name( snd_pcm_state( gpAudioDevice ) ) );

	snd_pcm_hw_params_get_channels( pParams, &ret );
	log_msg( "PCM channels:    %i ", ret );

	if ( ret == 1 )
		log_msg( "(mono)\n" );
	else if ( ret == 2 )
		log_msg( "(stereo)\n" );

	snd_pcm_hw_params_get_rate( pParams, &ret, 0 );
	log_msg( "PCM sample rate: %d bps\n", ret );

	return 1;
#endif /* USE_ALSA  */
return 1;
}

/*
 *    Cleans up the audio device.
 */
void audio_quit( void ) {
#if USE_ALSA
	//snd_pcm_drain( gpAudioDevice );
	snd_pcm_close( gpAudioDevice );
#endif /* USE_ALSA  */
}

/*
 *    Initializes SDL for presentation and input.
 */
u32 surface_init( void ) {
	s32 width  = args_get_int( "-w" );
    s32 height = args_get_int( "-h" );

	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		log_warn( "u32 surface_init( void ): SDL_Init() failed: %s\n", SDL_GetError() );
		return 0;
	}

    if ( width == -1 || height == -1 ) {
        width  = DEFAULT_WIDTH;
        height = DEFAULT_HEIGHT;
    }
#if USE_SDL
	/*
 	 *    Set the window title.
 	 */
    const s8 *pTitle = app_get_name();
    if ( pTitle == nullptr ) {
        pTitle = DEFAULT_TITLE;
    }

	/*
	 *    Create the window.
	 */
    gpWindow = SDL_CreateWindow( pTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    if( gpWindow == nullptr ) {
        log_error( "u32 surface_init( void ): Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
    }

	/*
	 *    Create the renderer.
	 */
    gpRenderer = SDL_CreateRenderer( gpWindow, -1, SDL_RENDERER_ACCELERATED );
    if( gpRenderer == nullptr ) {
        log_error( "u32 surface_init( void ): Renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
    }

	/*
	 *    Create the texture.
	 */
    gpTexture = SDL_CreateTexture( gpRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height );
    if( gpTexture == nullptr ) {
        log_error( "u32 surface_init( void ): Texture could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 0;
    }
#endif /* USE_SDL  */
	return 1;
}

/*
 *    Cleans up SDL.
 */
void surface_quit( void ) {
#if USE_SDL
	SDL_DestroyTexture( gpTexture );
	SDL_DestroyRenderer( gpRenderer );
	SDL_DestroyWindow( gpWindow );
	SDL_Quit();
#endif /* USE_SDL  */
}

/*
 *    Capture input events.
 */
u32 input_capture( void ) {
#if USE_SDL
	vec2u_t res = platform_get_screen_size();
	if ( ( SDL_GetWindowFlags( gpWindow ) & SDL_WINDOW_INPUT_FOCUS ) ) {
        SDL_SetRelativeMouseMode( SDL_TRUE );
		/*
		 *    Center the mouse.
		 */
        SDL_WarpMouseInWindow( gpWindow, res.x / 2, res.y / 2 );
    }
    else {
        SDL_SetRelativeMouseMode( SDL_FALSE );
    }
	SDL_PumpEvents();
	gpKeyState = SDL_GetKeyboardState( nullptr );
#endif /* USE_SDL  */
	return 1;
}

/*
 *    Parses the input event lookup.
 *
 *    @param const s8 *    The file with the lookup table.
 */
u32 input_parse( const s8 *spFile ) {
	u32 fileLen;
	s8 *pFile = file_read( spFile, &fileLen );
	u32 p = 0;
	u32 i = 0;

	if ( pFile == nullptr ) {
		log_error( "u32 input_parse( const s8 * ): Failed to read file: %s\n", spFile );
		return 0;
	}
	while ( p < fileLen ) {
		/*
		 *    Skip whitespace.
		 */
		while ( p < fileLen && pFile[ p ] <= ' ' ) {
			p++;
		}
		if ( p >= fileLen ) {
			break;
		}

		/*
		 *    Get the key.
		 */
		u32 key = 0;
		while ( p < fileLen && pFile[ p ] >= '0' && pFile[ p ] <= '9' ) {
			key = key * 10 + pFile[ p ] - '0';
			p++;
		}
		if ( p >= fileLen ) {
			break;
		}
		if ( key == 0 ) {
			log_error( "u32 input_parse( const s8 * ): Invalid key: %s\n", spFile );
			return 0;
		}
		gKeys[ i ] = key;

		/*
		 *    Skip whitespace then get the action.
		 */
		while ( p < fileLen && pFile[ p ] <= ' ' ) {
			p++;
		}

		u32 j = 0;
		while ( p < fileLen && pFile[ p ] >= 'A' && pFile[ p ] <= 'z' ) {
			gpKeyAliases[ i ][ j++ ] = pFile[ p++ ];
		}
		i++;

		/*
		 *    Skip whitespace.
		 */
		while ( p < fileLen && pFile[ p ] <= ' ' ) {
			p++;
		}
		if ( p >= fileLen ) {
			break;
		}

		/*
		 *    Check for semicolon.
		 */
		if ( pFile[ p ] != ';' ) {
			log_error( "u32 input_parse( const s8 * ): Invalid file format: %s\n", spFile );
			return 0;
		}
		p++;
	}

	file_free( pFile );
}

/*
 *    Draws a bitmap to the screen.
 *
 *    @param image_t *    The image to draw.
 *
 *    @return u32         1 if successful, 0 otherwise.
 */
u32 platform_draw_image( image_t *spImage ) {
#if USE_SDL
	SDL_RenderClear( gpRenderer );
    SDL_UpdateTexture( gpTexture, nullptr, spImage->apData, spImage->aWidth * sizeof( u32 ) );
	//SDL_RenderCopy( gpRenderer, gpTexture, nullptr, nullptr );
    SDL_RenderCopyEx( gpRenderer, gpTexture, nullptr, nullptr, 0.0, nullptr, SDL_FLIP_VERTICAL );
    SDL_RenderPresent( gpRenderer );
#endif /* USE_SDL  */
	return 1;
}

/*
 *    Returns the width and height of the screen.
 *
 *    @return vec2u_t      The width and height of the screen.
 */
vec2u_t platform_get_screen_size( void ) {
#if USE_SDL
	vec2u_t vSize;
	SDL_GetWindowSize( gpWindow, &vSize.x, &vSize.y );
	return vSize;
#endif /* USE_SDL  */
}

/*
 *    Pops an event from the input queue.
 *
 *    @param u32 *    Additional information about the event.
 *
 *    @return s8 *    The event, or nullptr if there are no events.
 */
s8 *platform_get_event( u32 *spInfo ) {
#if USE_SDL
	u64 i;
	for ( i = 0; i < MAX_INPUT_TYPES; ++i ) {
		if ( gpKeyState[ gKeys[ i ] ] && gKeyMask[ gKeys[ i ] ] == 0 ) {
			gKeyMask[ gKeys[ i ] ] = 1;
			return gpKeyAliases[ i ];
		}
	}
	return nullptr;
#endif /* USE_SDL  */
}

/*
 *    Returns a joystick event.
 *
 *    @return vec2u_t      The joystick event.
 */
vec2u_t platform_get_joystick_event() {
#if USE_SDL
	vec2u_t event = { 0, 0 };
	if ( ( SDL_GetWindowFlags( gpWindow ) & SDL_WINDOW_INPUT_FOCUS ) ) {
        SDL_GetRelativeMouseState( &event.x, &event.y );
    }
	return event;
#endif /* USE_SDL  */
}

/*
 *    Writes data to the sound buffer. The platform will
 *    read a specific amount of data from the buffer, and
 *    therefore the caller should ensure that the buffer
 *    is large enough.
 *
 *    @param s8 *     The data to write.
 * 
 *    @return u32     1 if successful, 0 otherwise.
 */
u32 platform_write_sound( s8 *spData ) {
#if USE_ALSA
	u32 ret;

	if ( ret = snd_pcm_writei( gpAudioDevice, spData, PCM_BUFFER_SIZE ) == -EPIPE ) {
		log_warn( "u32 platform_write_sound( s8 *spData ): Audio buffer can't keep up with sound playback!\n" );
		snd_pcm_prepare( gpAudioDevice );
		return 0;
	}
	else if ( ret < 0 ) {
		log_warn( "u32 platform_write_sound( s8 *spData ): Can't write to PCM device. %s\n", snd_strerror( ret ) );
		return 0;
	}
	return 1;
#endif /* USE_ALSA  */
}

/*
 *    Gets the playback bits per sample, sample rate, channels, and buffer size.
 *
 *    @param u32 *    The bits per sample.
 *    @param u32 *    The sample rate.
 *    @param u32 *    The channels.
 *    @param u32 *    The buffer size.
 */
void platform_get_sound_info( u32 *spBitsPerSample, u32 *spSampleRate, u32 *spChannels, u32 *spBufferSize ) {
	*spBitsPerSample = PCM_SAMPLE_WIDTH;
	*spSampleRate    = PCM_SAMPLE_RATE;
	*spChannels      = PCM_CHANNELS;
	*spBufferSize    = PCM_BUFFER_SIZE;
}

/*
 *    Reads from stdin.
 *
 *    @return s8 *    The string read from stdin.
 */
s8 *platform_read_stdin() {
#if __unix__
	static s8 buf[ MAX_STDIN_READ ] = { 0 };
	if ( read( 0, &buf, 1 ) > 0 ) {
		return buf;
	}
	return nullptr;
#elif _WIN32
#endif /* __unix__  */
}

/*
 *    Initializes the platform.
 *
 *    @return u32    1 if successful, 0 otherwise.
 */
u32 platform_init( void ) {
	gpPlatformResources = mempool_new( 1000000 );
	if( gpPlatformResources == nullptr ) {
		log_error( "u32 platform_init( void ): Unable to allocate memory pool for platform resources.\n" );
		return 0;
	}
    if ( !audio_init() ) {
		log_error( "u32 platform_init( void ): Unable to initialize audio.\n" );
		return 0;
	}
	if ( !surface_init() ) {
		log_error( "u32 platform_init( void ): Unable to initialize surface.\n" );
		return 0;
	}
	input_parse( "./aliases_sdl.txt" );

#if __unix__
	fcntl( 0, F_SETFL, O_NONBLOCK );
#endif /* __unix__  */
    
    return 1;
}

/*
 *    Updates the platform.
 *
 *    @param  f32    Delta time.
 *
 *    @return u32    1 if successful, 0 otherwise.
 */
u32 platform_update( f32 sDT ) {
	memset( gKeyMask, 0, sizeof( gKeyMask ) );
	input_capture();
    return 1;
}

/*
 *    Cleans up the platform.
 *
 *    @return u32    1 if successful, 0 otherwise.
 */
u32 platform_cleanup( void ) {
	if ( gpPlatformResources != nullptr ) {
		mempool_destroy( gpPlatformResources );
	}
	audio_quit();
	surface_quit();
    return 1;
}

CHIK_MODULE( platform_init, platform_update, platform_cleanup )