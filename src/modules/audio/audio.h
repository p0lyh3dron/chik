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

#define CHIK_AUDIO_TYPE_LOOP (1 << 0)
#define CHIK_AUDIO_TYPE_HRTF (1 << 1)

typedef struct {
    int            flags;
    unsigned char *data;
    unsigned int   samples;
    unsigned int   pos;
    unsigned int   playing;

    vec3_t listen_pos;
    vec3_t source_pos;
} audio_t;

/*
 *    Initializes an audio internally.
 *
 *    @return audio_t *    The audio pointer.
 */
audio_t *audio_ptr_init(void);

/*
 *    Populates a buffer with WAV data.
 *
 *    @param const char    *path    The path to the WAV file.
 *    @param unsigned long *samples     The number of samples.
 *
 *    @return unsigned char *         The buffer.
 */
unsigned char *audio_read_wav(const char *path, unsigned long *samples);

/*
 *    Creates an audio handle from a file on disk.
 *
 *    @param const char *path           The path to the audio file.
 *    @param unsigned int loop          Whether the audio should loop.
 *
 *    @return void *                    The handle to the audio file.
 */
void *audio_create_from_file(const char *path, unsigned int loop);

/*
 *    Plays an audio handle.
 *
 *    @param trap_t void *audio     The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully played.
 */
unsigned int audio_play(void *audio);

/*
 *    Stops an audio handle.
 *
 *    @param void *audio    The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully stopped.
 */
unsigned int audio_stop(void *audio);

/*
 *    Sets the listener position for HRTF audio.
 *
 *    @param void *audio            The handle to the audio file.
 *    @param vec3_t listen_pos      The position of the listener.
 *    @param vec3_t source_pos      The position of the sound source.
 *
 *    @return unsigned int         Whether the listener position was successfully set.
 */
unsigned int audio_set_listener_position(void *audio, vec3_t listen_pos, vec3_t source_pos);

#endif /* CHIK_AUDIO_H  */