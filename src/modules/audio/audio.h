/*
 *    audio.h    --    header for handling audio
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 17, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    This file declares audio-related stuff, such as
 *    music playback or HRTF audio.
 */
#ifndef CHIK_AUDIO_H
#define CHIK_AUDIO_H

#include "libchik.h"

#define CHIK_AUDIO_MAX_AUDIO_HANDLES 32

typedef enum {
    CHIK_AUDIO_TYPE_LOOP = 1 << 0,
    CHIK_AUDIO_TYPE_HRTF = 1 << 1,
} audio_flags_e;

typedef struct  {
    audio_flags_e  aFlags;
    u8            *apSamples;
    u32            aPos;
    u32            aPlaying;
} audio_t;

/*
 *    Initializes an audio internally.
 *
 *    @return audio_t *    The audio pointer.
 */
audio_t *audio_ptr_init( void );

/*
 *    Populates a buffer with WAV data.
 *
 *    @param const s8 *    The path to the WAV file.
 * 
 *    @return u8 *         The buffer.
 */
u8 *audio_read_wav( const s8 *spPath );

/*
 *    Creates an audio handle from a file on disk.
 *
 *    @param const s8 *    The path to the audio file.
 *    @param u32           Whether the audio should loop.
 * 
 *    @return trap_t     The handle to the audio file.
 */
trap_t audio_create_from_file( const s8 *spPath, u32 sLoop );

/*
 *    Plays an audio handle.
 *
 *    @param trap_t     The handle to the audio file.
 *    
 *    @return u32         Whether the audio was successfully played.
 */
u32 audio_play( trap_t sAudio );

/*
 *    Stops an audio handle.
 *
 *    @param trap_t     The handle to the audio file.
 * 
 *    @return u32         Whether the audio was successfully stopped.
 */
u32 audio_stop( trap_t sAudio );

/*
 *    Sets the listener position for HRTF audio.
 *
 *    @param vec3_t       The position of the listener.
 *    @param vec3_t       The position of the sound source.
 *    
 *    @return u32         Whether the listener position was successfully set.
 */
u32 audio_set_listener_position( vec3_t sListenerPosition, vec3_t sSoundSourcePosition );

#endif /* CHIK_AUDIO_H  */