/*
 *    audio.c    --    source for handling audio
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 17, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    This file defines audio-related stuff, such as
 *    music playback or HRTF audio.
 */
#include "audio.h"

u32 gSampleWidth = 0;
u32 gSampleRate  = 0;
u32 gNumChannels = 0;
u32 gNumSamples  = 0;

mempool_t  *gpAudioPool      = nullptr;
resource_t *gpAudioResources = nullptr;

audio_t    *gpAudioPtrs[ CHIK_AUDIO_MAX_AUDIO_HANDLES ] = { nullptr };

u8 *gpAudioBuffer = nullptr;

u32 audio_init( void );

u32 audio_update( f32 sDT );

u32 audio_shutdown( void );

CHIK_MODULE( audio_init, audio_update, audio_shutdown )

u32  ( *platform_write_sound )( s8 *spData ) = 0;
void ( *platform_get_sound_info )( u32 *spBitsPerSample, u32 *spSampleRate, u32 *spChannels, u32 *spBufferSize ) = 0;

u32 audio_init( void ) {
    *( void** )( &platform_write_sound )    = engine_load_function( "platform_write_sound" );
    *( void** )( &platform_get_sound_info ) = engine_load_function( "platform_get_sound_info" );
    if ( platform_write_sound == nullptr ) {
        log_error( "u32 audio_init( void ): Failed to find platform function for writing audio samples!\n" );
        return 0;
    }

    if ( platform_get_sound_info == nullptr ) {
        log_error( "u32 audio_init( void ): Failed to find platform function for getting sound info!\n" );
        return 0;
    }

    gpAudioPool = mempool_new( 1024 * 1024 );
    if ( gpAudioPool == nullptr ) {
        log_error( "u32 audio_init( void ): Failed to create audio pool!\n" );
        return 0;
    }

    gpAudioResources = resource_new( CHIK_AUDIO_MAX_AUDIO_HANDLES * sizeof( handle_t ) );
    if ( gpAudioResources == nullptr ) {
        log_error( "u32 audio_init( void ): Failed to create audio resources!\n" );
        return 0;
    }

    platform_get_sound_info( &gSampleWidth, &gSampleRate, &gNumChannels, &gNumSamples );
    gpAudioBuffer = ( u8 * )mempool_alloc( gpAudioPool, gSampleWidth * gNumChannels * gNumSamples );
    if ( gpAudioBuffer == nullptr ) {
        log_error( "u32 audio_init( void ): Failed to allocate audio buffer!\n" );
        return 0;
    }

    return 1;
}

u32 audio_update( f32 sDT ) {
    /*
     *    Write audio data to the sound buffer.
     */
    u64 i;
    for ( i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; ++i ) {
        if ( gpAudioPtrs[ i ] != nullptr ) {
            audio_t *pAudio = gpAudioPtrs[ i ];
            if ( pAudio->apSamples != nullptr ) {
                
            }
        }
    }
    //platform_write_sound( gpAudioBuffer );
    return 1;
}

u32 audio_shutdown( void ) {
    mempool_destroy( gpAudioPool );
    resource_destroy( gpAudioResources );

    return 1;
}

/*
 *    Initializes an audio internally.
 *
 *    @return audio_t *    The audio pointer.
 */
audio_t *audio_ptr_init( void ) {
    audio_t *pAudio = ( audio_t* )mempool_alloc( gpAudioPool, sizeof( audio_t ) );

    if ( pAudio == nullptr ) {
        log_error( "audio_ptr_init( void ): Failed to allocate audio!\n" );
        return nullptr;
    }
    pAudio->aFlags    = 0;
    pAudio->apSamples = nullptr;
    pAudio->aPlaying  = 0;
    pAudio->aPos      = 0;

    u64 i;
    for ( i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; i++ ) {
        if ( gpAudioPtrs[ i ] == nullptr ) {
            gpAudioPtrs[ i ] = pAudio;
            return pAudio;
        }
    }

    log_error( "audio_ptr_init( void ): Failed to allocate audio!\n" );
    return nullptr;
}

/*
 *    Populates a buffer with WAV data.
 *
 *    @param const s8 *    The path to the WAV file.
 * 
 *    @return u8 *         The buffer.
 */
u8 *audio_read_wav( const s8 *spPath ) {
    u32 sSize = 0;
    u8 *pData = file_read( spPath, &sSize );
    if ( pData == nullptr ) {
        log_error( "audio_read_wav( const s8 * ): Failed to read WAV file!\n" );
        return nullptr;
    }

    /*
     *    Implement WAV file parsing here.
     */

    return pData;
}

/*
 *    Creates an audio handle from a file on disk.
 *
 *    @param const s8 *    The path to the audio file.
 *    @param u32           Whether the audio should loop.
 * 
 *    @return handle_t     The handle to the audio file.
 */
handle_t audio_create_from_file( const s8 *spPath, u32 sLoop ) {
    audio_t *pA = audio_ptr_init();
    if ( pA == nullptr ) {
        log_error( "audio_create_from_file( const s8 *, u32 ): Failed to allocate audio!\n" );
        return 0;
    }
    pA->aFlags    = sLoop;
    pA->apSamples = audio_read_wav( spPath );

    if ( pA->apSamples == nullptr ) {
        log_error( "audio_create_from_file( const s8 *, u32 ): Failed to read audio file!\n" );
        return 0;
    }

    handle_t h = resource_add( gpAudioResources, &pA, sizeof( pA ) );

    if ( h == INVALID_HANDLE ) {
        log_error( "audio_create_from_file( const s8 *, u32 ): Failed to add audio to resources!\n" );
    }

    return h;
}

/*
 *    Plays an audio handle.
 *
 *    @param handle_t     The handle to the audio file.
 *    
 *    @return u32         Whether the audio was successfully played.
 */
u32 audio_play( handle_t sAudio ) {
    audio_t *pA = resource_get( gpAudioResources, sAudio );
    if ( pA == nullptr ) {
        log_error( "audio_play( handle_t ): Failed to get audio from resources!\n" );
        return 0;
    }
    pA->aPlaying = 1;
    return 1;
}

/*
 *    Stops an audio handle.
 *
 *    @param handle_t     The handle to the audio file.
 * 
 *    @return u32         Whether the audio was successfully stopped.
 */
u32 audio_stop( handle_t sAudio ) {
    audio_t *pA = resource_get( gpAudioResources, sAudio );
    if ( pA == nullptr ) {
        log_error( "audio_stop( handle_t ): Failed to get audio from resources!\n" );
        return 0;
    }
    pA->aPlaying = 0;
    return 1;
}

/*
 *    Sets the listener position for HRTF audio.
 *
 *    @param vec3_t       The position of the listener.
 *    @param vec3_t       The position of the sound source.
 *    
 *    @return u32         Whether the listener position was successfully set.
 */
u32 audio_set_listener_position( vec3_t sListenerPosition, vec3_t sSoundSourcePosition ) {

}

