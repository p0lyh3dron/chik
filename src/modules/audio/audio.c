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

double _audio_time = 0.0;

audio_t *_audio[CHIK_AUDIO_MAX_AUDIO_HANDLES] = {(audio_t *)0x0};

unsigned char *_audio_buf = (unsigned char *)0x0;

unsigned int audio_init(void);

unsigned int audio_update(float dt);

unsigned int audio_shutdown(void);

CHIK_MODULE(audio_init, audio_update, audio_shutdown)

unsigned int (*platform_write_sound)(char *)                                                    = 0;
void (*platform_get_sound_info)(unsigned int *, unsigned int *, unsigned int *, unsigned int *) = 0;

unsigned int audio_init(void) {
    *(void **)(&platform_write_sound)    = engine_load_function("platform_write_sound");
    *(void **)(&platform_get_sound_info) = engine_load_function("platform_get_sound_info");

    if (platform_write_sound == (void *)0x0) {
        LOGF_ERR("Failed to find platform function for writing audio samples!\n");

        return 0;
    }

    if (platform_get_sound_info == (void *)0x0) {
        LOGF_ERR("Failed to find platform function for getting sound info!\n");

        return 0;
    }

    platform_get_sound_info(&_sample_width, &_sample_rate, &_num_channels, &_num_samples);

    _audio_buf = (unsigned char *)malloc(_sample_width * _num_channels * _num_samples);

    if (_audio_buf == (unsigned char *)0x0) {
        LOGF_ERR("Failed to allocate audio buffer!\n");

        return 0;
    }

    return 1;
}

unsigned int audio_update(float dt) {
    /* Clear the audio buffer.  */
    memset(_audio_buf, 0, _sample_width * _num_channels * _num_samples);
    /* Write audio data to the sound buffer.  */
    size_t i;
    for (i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; ++i) {
        if (_audio[i] != nullptr) {
            audio_t *audio = _audio[i];
            if (audio->data != (unsigned char*)0x0) {
                /* Simply add the audio data to the buffer.  */
                if (audio->flags & CHIK_AUDIO_TYPE_LOOP && 0) {
                    size_t j;
                    for (j = 0; j < _num_samples * 4; j += _sample_width / 8) {
                        short *buf = (short *)(_audio_buf + j);
                        *buf += *(short*)(audio->data + audio->pos * _sample_width / 8 + j);
                    }
                }
                else {
                    float  ear_dist = 0.5;
                    vec2_t ear_strength;

                    /* Calculate the volume in each ear due to distance from source.  */
                    ear_strength.x = pow(audio->source_pos.x - audio->listen_pos.x - ear_dist * cos(audio->direction.y), 2) +
                                     pow(audio->source_pos.z - audio->listen_pos.z - ear_dist * sin(audio->direction.y), 2) +
                                     pow(audio->source_pos.y - audio->listen_pos.y, 2);

                    ear_strength.y = pow(audio->source_pos.x - audio->listen_pos.x + ear_dist * cos(audio->direction.y), 2) +
                                     pow(audio->source_pos.z - audio->listen_pos.z + ear_dist * sin(audio->direction.y), 2) +
                                     pow(audio->source_pos.y - audio->listen_pos.y, 2);

                    ear_strength.x = 1.0 / ear_strength.x * 4;
                    ear_strength.y = 1.0 / ear_strength.y * 4;

                    ear_strength.x = MIN(ear_strength.x, 1.0);
                    ear_strength.y = MIN(ear_strength.y, 1.0);

                    float         strength;
                    char          left;
                    size_t j;
                    for (j = left = 0; j < _num_samples * 4; j += _sample_width / 8) {
                        strength = left ? ear_strength.y : ear_strength.x;

                        short *buf = (short *)(_audio_buf + j);
                        *buf += strength * (*(short*)(audio->data + audio->pos * _sample_width / 8 + j));

                        left = !left;
                    }
                }
            }

            _audio[i]->pos += _sample_rate * _sample_width / 8 * dt;
        }
    }
    platform_write_sound(_audio_buf);

    return 1;
}

unsigned int audio_shutdown(void) {
    free(_audio_buf);

    return 1;
}

/*
 *    Initializes an audio internally.
 *
 *    @return audio_t *    The audio pointer.
 */
audio_t *audio_ptr_init(void) {
    size_t i;
    audio_t      *audio = (audio_t *)malloc(sizeof(audio_t));

    if (audio == (audio_t *)0x0) {
        LOGF_ERR("Failed to allocate audio!\n");
        return nullptr;
    }
    audio->flags   = 0;
    audio->data    = (unsigned char *)0x0;
    audio->playing = 0;
    audio->pos     = 0;

    for (i = 0; i < CHIK_AUDIO_MAX_AUDIO_HANDLES; i++) {
        if (_audio[i] == (audio_t *)0x0) {
            _audio[i] = audio;
            return audio;
        }
    }

    LOGF_ERR("Failed to allocate audio!\n");
    return nullptr;
}

/*
 *    Populates a buffer with WAV data.
 *
 *    @param const char    *path    The path to the WAV file.
 *    @param size_t *samples     The number of samples.
 *
 *    @return unsigned char *         The buffer.
 */
unsigned char *audio_read_wav(const char *path, size_t *samples) {
    unsigned int   len  = 0;
    unsigned char *data = file_read(path, &len);

    if (data == (unsigned char *)0x0) {
        LOGF_ERR("Failed to read WAV file!\n");

        return (unsigned char *)0x0;
    }

    size_t pos = 0;

    /* Check the RIFF header.  */
    if (data[pos++] != 'R' || data[pos++] != 'I' || data[pos++] != 'F' || data[pos++] != 'F') {
        LOGF_ERR("Invalid RIFF header!\n");

        return (unsigned char *)0x0;
    }

    /* Get the size of the file.  */
    unsigned int size = *(unsigned int *)(data + pos);
    pos += 4;

    /* Check the WAVE header.  */
    if (data[pos++] != 'W' || data[pos++] != 'A' || data[pos++] != 'V' || data[pos++] != 'E') {
        LOGF_ERR("Invalid WAVE header!\n");

        return (unsigned char *)0x0;
    }

    /* Check the fmt header.  */
    if (data[pos++] != 'f' || data[pos++] != 'm' || data[pos++] != 't' || data[pos++] != ' ') {
        LOGF_ERR("Invalid fmt header!\n");

        return (unsigned char *)0x0;
    }

    /* Get the size of the fmt chunk.  */
    unsigned int fmt_size = *(unsigned int *)(data + pos);
    pos += 4;

    /* Get the audio format.  */
    unsigned short format = *(unsigned short *)(data + pos);
    pos += 2;

    /* Get the number of channels.  */
    unsigned short channels = *(unsigned short *)(data + pos);
    pos += 2;

    /* Get the sample rate.  */
    unsigned int sample_rate = *(unsigned int *)(data + pos);
    pos += 4;

    /* Get the byte rate.  */
    unsigned int byte_rate = *(unsigned int *)(data + pos);
    pos += 4;

    /* Get the block align.  */
    unsigned short block_align = *(unsigned short *)(data + pos);
    pos += 2;

    /* Get the bits per sample.  */
    unsigned short bits_per_sample = *(unsigned short *)(data + pos);
    pos += 2;

    /* Check the data header.  */
    if (data[pos++] != 'd' || data[pos++] != 'a' || data[pos++] != 't' || data[pos++] != 'a') {
        LOGF_ERR("Invalid data header!\n");

        return (unsigned char *)0x0;
    }

    /* Get the size of the data chunk.  */
    unsigned int data_size = *(unsigned int *)(data + pos);
    pos += 4;

    /* Extend buffer if it is not stereo.  */
    if (channels == 1)
        data_size *= 2;

    /* Check the audio format.  */
    if (format != 1) {
        LOGF_ERR("Invalid audio format!\n");

        return (unsigned char *)0x0;
    }

    unsigned char *audio_data = (unsigned char *)malloc(data_size);

    size_t idx;
    while (pos < len) {
        if (bits_per_sample == 8) {
            unsigned short sample = (unsigned short)data[pos++];
            memcpy(audio_data + idx, &sample, 2);

            /* Copy to stereo if mono.  */
            if (channels == 1) {
                memcpy(audio_data + idx + 2, &sample, 2);

                idx += 2;
            }
        }
        else if (bits_per_sample == 16) {
            unsigned short sample = *(unsigned short *)(data + pos);
            pos += 2;
            memcpy(audio_data + idx, &sample, 2);

            /* Copy to stereo if mono.  */
            if (channels == 1) {
                memcpy(audio_data + idx + 2, &sample, 2);

                idx += 2;
            }
        }

        idx += 2;
    }

    *samples = data_size / channels / (bits_per_sample / 8);

    return audio_data;
}

/*
 *    Creates an audio handle from a file on disk.
 *
 *    @param const char *path           The path to the audio file.
 *    @param unsigned int loop          Whether the audio should loop.
 *
 *    @return void *                    The handle to the audio file.
 */
void *audio_create_from_file(const char *path, unsigned int loop) {
    audio_t *a = audio_ptr_init();

    if (a == (audio_t *)0x0) {
        LOGF_ERR("Failed to allocate audio!\n");

        return a;
    }

    a->flags = loop;
    a->data  = audio_read_wav(path, &a->samples);

    if (a->data == (unsigned char *)0x0) {
        LOGF_ERR("Failed to read audio file!\n");

        return (unsigned char *)0x0;
    }

    return a;
}

/*
 *    Plays an audio handle.
 *
 *    @param trap_t void *audio     The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully played.
 */
unsigned int audio_play(void *audio) {
    if (audio == (void *)0x0) {
        LOGF_ERR("Failed to get audio from resources!\n");

        return 0;
    }

    audio_t *a = (audio_t *)audio;

    a->playing = 1;

    return 1;
}

/*
 *    Stops an audio handle.
 *
 *    @param void *audio    The handle to the audio file.
 *
 *    @return unsigned int         Whether the audio was successfully stopped.
 */
unsigned int audio_stop(void *audio) {
    if (audio == (void *)0x0) {
        LOGF_ERR("Failed to get audio from resources!\n");

        return 0;
    }

    audio_t *a = (audio_t *)audio;

    a->playing = 0;

    return 1;
}

/*
 *    Sets the listener position for HRTF audio.
 *
 *    @param void *audio            The handle to the audio file.
 *    @param vec3_t listen_pos      The position of the listener.
 *    @param vec3_t source_pos      The position of the sound source.
 *    @param vec2_t direction       The direction of the listener.
 *
 *    @return unsigned int         Whether the listener position was successfully set.
 */
unsigned int audio_set_listener_position(void *audio, vec3_t listen_pos, vec3_t source_pos, vec2_t direction) {
    if (audio == (void *)0x0) {
        //LOGF_ERR("Failed to get audio from resources!\n");

        return 0;
    }

    audio_t *a = (audio_t *)audio;

    a->listen_pos = listen_pos;
    a->source_pos = source_pos;
    a->direction  = direction;

    return 1;
}