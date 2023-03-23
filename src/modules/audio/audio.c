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

unsigned int _sample_width = 0;
unsigned int _sample_rate  = 0;
unsigned int _num_channels = 0;
unsigned int _num_samples  = 0;

resource_t *_audio_resources = nullptr;

audio_t *_audio[CHIK_AUDIO_MAX_AUDIO_HANDLES] = {nullptr};

unsigned char *_audio_buf = nullptr;

unsigned int audio_init(void);

unsigned int audio_update(float dt);

unsigned int audio_shutdown(void);

CHIK_MODULE(audio_init, audio_update, audio_shutdown)

unsigned int (*platform_write_sound)(char *)                                                    = 0;
void (*platform_get_sound_info)(unsigned int *, unsigned int *, unsigned int *, unsigned int *) = 0;

unsigned int audio_init(void) {
    *(void **)(&platform_write_sound) =
        engine_load_function("platform_write_sound");
    *(void **)(&platform_get_sound_info) =
        engine_load_function("platform_get_sound_info");

    if (platform_write_sound == nullptr) {
        LOGF_ERR("Failed to find platform function "
                 "for writing audio samples!\n");
        return 0;
    }

    if (platform_get_sound_info == nullptr) {
        LOGF_ERR("Failed to find platform function "
                 "for getting sound info!\n");
        return 0;
    }

    _audio_resources =
        resource_new(CHIK_AUDIO_MAX_AUDIO_HANDLES * sizeof(trap_t));
    if (_audio_resources == nullptr) {
        LOGF_ERR("Failed to create audio resources!\n");
        return 0;
    }

    platform_get_sound_info(&_sample_width, &_sample_rate, &_num_channels,
                            &_num_samples);
    _audio_buf = (unsigned char *)malloc(_sample_width * _num_channels * _num_samples);
    if (_audio_buf == nullptr) {
        LOGF_ERR("Failed to allocate audio buffer!\n");
        return 0;
    }

    return 1;
}

unsigned int audio_update(float dt) {
    unsigned long i;
    audio_t      *audio;

    /*
     *    Write audio data to the sound buffer.
     */
    for (i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; ++i) {
        if (_audio[i] != nullptr) {
            audio = _audio[i];
            if (audio->data != nullptr) {
            }
        }
    }
    // platform_write_sound( _audio_buf );
    return 1;
}

unsigned int audio_shutdown(void) {
    free(_audio_buf);
    resource_destroy(_audio_resources);

    return 1;
}

/*
 *    Initializes an audio internally.
 *
 *    @return audio_t *    The audio pointer.
 */
audio_t *audio_ptr_init(void) {
    unsigned long i;
    audio_t      *pAudio = (audio_t *)malloc(sizeof(audio_t));

    if (pAudio == nullptr) {
        LOGF_ERR("Failed to allocate audio!\n");
        return nullptr;
    }
    pAudio->flags   = 0;
    pAudio->data    = nullptr;
    pAudio->playing = 0;
    pAudio->pos     = 0;

    for (i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; i++) {
        if (_audio[i] == nullptr) {
            _audio[i] = pAudio;
            return pAudio;
        }
    }

    LOGF_ERR("Failed to allocate audio!\n");
    return nullptr;
}

/*
 *    Populates a buffer with WAV data.
 *
 *    @param const char *path    The path to the WAV file.
 *
 *    @return unsigned char *         The buffer.
 */
unsigned char *audio_read_wav(const char *path) {
    unsigned int   len  = 0;
    unsigned char *data = file_read(path, &len);

    if (data == nullptr) {
        LOGF_ERR("Failed to read WAV file!\n");
        return nullptr;
    }

    /*
     *    Implement WAV file parsing here.
     */

    return data;
}

/*
 *    Creates an audio handle from a file on disk.
 *
 *    @param const char *path    The path to the audio file.
 *    @param unsigned int loop          Whether the audio should loop.
 *
 *    @return trap_t     The handle to the audio file.
 */
trap_t audio_create_from_file(const char *path, unsigned int loop) {
    trap_t   h;
    audio_t *a = audio_ptr_init();

    if (a == nullptr) {
        LOGF_ERR("Failed to "
                 "allocate audio!\n");
        return INVALID_TRAP;
    }

    a->flags = loop;
    a->data  = audio_read_wav(path);

    if (a->data == nullptr) {
        LOGF_ERR("Failed to read "
                 "audio file!\n");
        return INVALID_TRAP;
    }

    h = resource_add(_audio_resources, &a, sizeof(a));

    if (BAD_TRAP(h)) {
        LOGF_ERR("Failed to add "
                 "audio to resources!\n");
    }

    return h;
}

/*
 *    Plays an audio handle.
 *
 *    @param trap_t audio     The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully played.
 */
unsigned int audio_play(trap_t audio) {
    audio_t *a = resource_get(_audio_resources, audio);

    if (a == nullptr) {
        LOGF_ERR("Failed to get audio from resources!\n");
        return 0;
    }

    a->playing = 1;
    return 1;
}

/*
 *    Stops an audio handle.
 *
 *    @param trap_t audio    The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully stopped.
 */
unsigned int audio_stop(trap_t audio) {
    audio_t *a = resource_get(_audio_resources, audio);

    if (a == nullptr) {
        LOGF_ERR("Failed to get audio from resources!\n");
        return 0;
    }

    a->playing = 0;
    return 1;
}

/*
 *    Sets the listener position for HRTF audio.
 *
 *    @param vec3_t listen_pos      The position of the listener.
 *    @param vec3_t source_pos      The position of the sound source.
 *
 *    @return unsigned int         Whether the listener position was successfully set.
 */
unsigned int audio_set_listener_position(vec3_t listen_pos, vec3_t source_pos) {}
