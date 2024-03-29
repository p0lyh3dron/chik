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
#include "libchik.h"

#if __unix__
#include <fcntl.h>
#endif /* __unix__  */

#if USE_ALSA
#include <alsa/asoundlib.h>
#endif /* USE_ALSA  */

#if USE_SDL
#include <SDL.h>
#endif /* USE_SDL  */

#define PCM_DEVICE       "default"
#define PCM_CHANNELS     2
#define PCM_SAMPLE_RATE  48000
#define PCM_BUFFER_SIZE  8192
#define PCM_SAMPLE_WIDTH 16
#define PCM_WRITE_SIZE   PCM_BUFFER_SIZE / PCM_CHANNELS * PCM_SAMPLE_WIDTH / 8

#define DEFAULT_WIDTH  1920
#define DEFAULT_HEIGHT 1080
#define DEFAULT_TITLE  "Chik Application"

#define MAX_INPUT_TYPES  256
#define MAX_ALIAS_LENGTH 32

#define MAX_STDIN_READ 256

unsigned int _keys[MAX_INPUT_TYPES]                        = {0};
char         _key_alias[MAX_INPUT_TYPES][MAX_ALIAS_LENGTH] = {{'\0'}};

#if USE_ALSA
snd_pcm_t *_aud_dev = nullptr;
#endif /* USE_ALSA  */

#if USE_SDL
SDL_Window   *_win  = nullptr;
SDL_Renderer *_rend = nullptr;
SDL_Texture  *_tex  = nullptr;

const char *_key_state                   = nullptr;
char        _key_mask[SDL_NUM_SCANCODES] = {0};
#endif /* USE_SDL  */

vec2u_t platform_get_screen_size(void);

/*
 *    Initialize the audio device.
 */
unsigned int audio_init(void) {
#if USE_ALSA
    unsigned int         rate     = PCM_SAMPLE_RATE;
    unsigned int         channels = PCM_CHANNELS;
    snd_pcm_hw_params_t *pParams  = nullptr;
    unsigned int         ret;

    /*
     *    Open the PCM device in playback mode
     */
    if (ret = snd_pcm_open(&_aud_dev, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) <
              0) {
        VLOGF_WARN("Can't open \"%s\" PCM device. %s\n", PCM_DEVICE,
                   snd_strerror(ret));
        return 1;
    }

    /*
     *    Allocate parameters and set defaults.
     */
    snd_pcm_hw_params_alloca(&pParams);
    snd_pcm_hw_params_any(_aud_dev, pParams);

    /*
     *    Using interleaved, we store a sample in one channel followed by
     * another in the other.
     */
    if (ret = snd_pcm_hw_params_set_access(_aud_dev, pParams,
                                           SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        VLOGF_WARN("Can't set interleaved mode. %s\n", snd_strerror(ret));
        return 1;
    }

    /*
     *    Set the sample format to be 16 bits.
     */
    if (ret = snd_pcm_hw_params_set_format(_aud_dev, pParams,
                                           SND_PCM_FORMAT_S16_LE) < 0) {
        VLOGF_WARN("Can't set format. %s\n", snd_strerror(ret));
        return 1;
    }

    /*
     *    Set the number of channels to be 2.
     */
    if (ret = snd_pcm_hw_params_set_channels(_aud_dev, pParams, channels) < 0) {
        VLOGF_WARN("Can't set channels number. %s\n", snd_strerror(ret));
        return 1;
    }

    /*
     *    Set the sample rate to be 44100 Hz.
     */
    if (ret =
            snd_pcm_hw_params_set_rate_near(_aud_dev, pParams, &rate, 0) < 0) {
        VLOGF_WARN("Can't set rate. %s\n", snd_strerror(ret));
        return 1;
    }

    snd_pcm_hw_params_set_buffer_size(_aud_dev, pParams, PCM_BUFFER_SIZE);

    /*
     *    Apply the hardware parameters to the PCM device.
     */
    if (ret = snd_pcm_hw_params(_aud_dev, pParams) < 0) {
        VLOGF_WARN("Can't set hardware parameters. %s\n", snd_strerror(ret));
        return 1;
    }

    /*
     *    Display the current hardware parameters.
     */
    VLOGF_MSG("PCM name:       '%s'\n", snd_pcm_name(_aud_dev));
    VLOGF_MSG("PCM state:       %s\n",
              snd_pcm_state_name(snd_pcm_state(_aud_dev)));

    snd_pcm_hw_params_get_channels(pParams, &ret);
    VLOGF_MSG("PCM channels:    %i ", ret);

    if (ret == 1)
        LOGF_MSG("(mono)\n");
    else if (ret == 2)
        LOGF_MSG("(stereo)\n");

    snd_pcm_hw_params_get_rate(pParams, &ret, 0);
    VLOGF_MSG("PCM sample rate: %d bps\n", ret);

    return 1;
#endif /* USE_ALSA  */
    return 1;
}

/*
 *    Cleans up the audio device.
 */
void audio_quit(void) {
#if USE_ALSA
    // snd_pcm_drain( _aud_dev );
    snd_pcm_close(_aud_dev);
#endif /* USE_ALSA  */
}

/*
 *    Initializes SDL for presentation and input.
 */
unsigned int surface_init(void) {
    int         width  = args_get_int("-w");
    int         height = args_get_int("-h");
    const char *pTitle = app_get_name();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        VLOGF_FAT("SDL_Init() failed: %s\n", SDL_GetError());
        return 0;
    }

    if (width == -1 || height == -1) {
        width  = DEFAULT_WIDTH;
        height = DEFAULT_HEIGHT;
    }
#if USE_SDL
    /*
     *    Set the window title.
     */
    if (pTitle == nullptr) {
        pTitle = DEFAULT_TITLE;
    }

    /*
     *    Create the window.
     */
    _win = SDL_CreateWindow(pTitle, 0,
                            0, width, height,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (_win == nullptr) {
        VLOGF_ERR("Window could not be created! "
                  "SDL_Error: %s\n",
                  SDL_GetError());
        return 0;
    }

    if (args_has("--software-renderer")) {
        /*
         *    Create the renderer.
         */
        _rend = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);

        if (_rend == nullptr) {
            VLOGF_ERR("Renderer could not be created! "
                      "SDL_Error: %s\n",
                      SDL_GetError());
            return 0;
        }

        /*
         *    Create the texture.
         */
        _tex = SDL_CreateTexture(_rend, SDL_PIXELFORMAT_RGB24,
                                 SDL_TEXTUREACCESS_STREAMING, width, height);
        if (_tex == nullptr) {
            VLOGF_ERR("Texture could not be created! "
                      "SDL_Error: %s\n",
                      SDL_GetError());
            return 0;
        }
    }
#endif /* USE_SDL  */
    return 1;
}

#if USE_SDL
/*
 *    Returns the SDL window.
 */
SDL_Window *surface_get_window(void) {
    return _win;
}
#endif /* USE_SDL  */

/*
 *    Cleans up SDL.
 */
void surface_quit(void) {
#if USE_SDL
    SDL_DestroyTexture(_tex);
    SDL_DestroyRenderer(_rend);
    SDL_DestroyWindow(_win);
    SDL_Quit();
#endif /* USE_SDL  */
}

/*
 *    Sets SDL window size.
 *
 *    @param vec2u_t size    The size to set.
 */
void surface_set_size(vec2u_t size) {
#if USE_SDL
    SDL_SetWindowSize(_win, size.x, size.y);
#endif /* USE_SDL  */
}

static vec2u_t gMouseDelta;

/*
 *    Capture input events.
 */
unsigned int input_capture(void) {
#if USE_SDL
    vec2u_t res = platform_get_screen_size();

    if ((SDL_GetWindowFlags(_win) & SDL_WINDOW_INPUT_FOCUS)) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        /*
         *    Center the mouse.
         */
        SDL_WarpMouseInWindow(_win, res.x / 2, res.y / 2);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    SDL_PumpEvents();
    _key_state = SDL_GetKeyboardState(nullptr);

    // get total event count first
    int eventCount = SDL_PeepEvents(nullptr, 0, SDL_PEEKEVENT, SDL_FIRSTEVENT,
                                SDL_LASTEVENT);

    // resize event vector with events found
    SDL_Event *events = malloc(sizeof(SDL_Event) * eventCount);

    // fill event vector with events found
    SDL_PeepEvents(events, eventCount, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

    gMouseDelta.x = 0;
    gMouseDelta.y = 0;

    for (int i = 0; i < eventCount; i++) {
        SDL_Event event = events[i];

        switch (event.type) {
            case SDL_MOUSEMOTION: {
              // aMousePos.x = event.motion.x;
              // aMousePos.y = event.motion.y;
              gMouseDelta.x += event.motion.xrel;
              gMouseDelta.y += event.motion.yrel;
              break;
            }
        }
    }

    free(events);

#endif /* USE_SDL  */
    return 1;
}

/*
 *    Parses the input event lookup.
 *
 *    @param const char *file    The file with the lookup table.
 */
unsigned int input_parse(const char *file) {
    unsigned int fileLen;
    char        *pFile = file_read(file, &fileLen);
    unsigned int p     = 0;
    unsigned int i     = 0;
    unsigned int j;
    unsigned int key;

    if (pFile == nullptr) {
        VLOGF_ERR("unsigned int input_parse( const char * ): Failed to read file: %s\n",
                  file);
        return 0;
    }

    while (p < fileLen) {
        /*
         *    Skip whitespace.
         */
        while (p < fileLen && pFile[p] <= ' ')
            p++;

        if (p >= fileLen)
            break;

        /*
         *    Get the key.
         */
        key = 0;

        while (p < fileLen && pFile[p] >= '0' && pFile[p] <= '9') {
            key = key * 10 + pFile[p] - '0';
            p++;
        }

        if (p >= fileLen)
            break;

        if (key == 0) {
            VLOGF_ERR("Invalid key: %s\n", file);
            return 0;
        }

        _keys[i] = key;

        /*
         *    Skip whitespace then get the action.
         */
        while (p < fileLen && pFile[p] <= ' ')
            p++;

        j = 0;

        while (p < fileLen && pFile[p] >= 'A' && pFile[p] <= 'z')
            _key_alias[i][j++] = pFile[p++];

        i++;

        /*
         *    Skip whitespace.
         */
        while (p < fileLen && pFile[p] <= ' ')
            p++;

        if (p >= fileLen)
            break;

        /*
         *    Check for semicolon.
         */
        if (pFile[p] != ';') {
            VLOGF_ERR("Invalid file format: %s\n", file);
            return 0;
        }

        p++;
    }

    file_free(pFile);
}

/*
 *    Draws a bitmap to the screen.
 *
 *    @param image_t *image    The image to draw.
 *
 *    @return unsigned int         1 if successful, 0 otherwise.
 */
unsigned int platform_draw_image(image_t *image) {
#if USE_SDL
    SDL_RenderClear(_rend);
    SDL_UpdateTexture(_tex, nullptr, image->buf, image->width * _pixel_sizes[image->fmt]);
    SDL_RenderCopyEx(_rend, _tex, nullptr, nullptr, 0.0, nullptr,
                     SDL_FLIP_VERTICAL);
    SDL_RenderPresent(_rend);
#endif /* USE_SDL  */
    return 1;
}

/*
 *    Returns the width and height of the screen.
 *
 *    @return vec2u_t      The width and height of the screen.
 */
vec2u_t platform_get_screen_size(void) {
#if USE_SDL
    vec2u_t vSize;

    SDL_GetWindowSize(_win, &vSize.x, &vSize.y);

    return vSize;
#endif /* USE_SDL  */
}

/*
 *    Pops an event from the input queue.
 *
 *    @param unsigned int *info    Additional information about the event.
 *
 *    @return char *    The event, or nullptr if there are no events.
 */
char *platform_get_event(unsigned int *info) {
#if USE_SDL
    size_t i;

    for (i = 0; i < MAX_INPUT_TYPES; ++i) {
        if (_key_state[_keys[i]] && _key_mask[_keys[i]] == 0) {
            _key_mask[_keys[i]] = 1;
            return _key_alias[i];
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
    vec2u_t event = {0, 0};

    if ((SDL_GetWindowFlags(_win) & SDL_WINDOW_INPUT_FOCUS))
        SDL_GetRelativeMouseState(&event.x, &event.y);

    return event;
#endif /* USE_SDL  */
}

/*
 *    Writes data to the sound buffer. The platform will
 *    read a specific amount of data from the buffer, and
 *    therefore the caller should ensure that the buffer
 *    is large enough.
 *
 *    @param char *buf     The data to write.
 *
 *    @return unsigned int     1 if successful, 0 otherwise.
 */
unsigned int platform_write_sound(char *buf) {
#if USE_ALSA
    unsigned int ret;

    if ((ret = snd_pcm_avail_update(_aud_dev)) > 2048) {
        if (ret = snd_pcm_writei(_aud_dev, buf, PCM_WRITE_SIZE) == -EPIPE) {
            LOGF_WARN("Audio buffer can't "
                    "keep up with sound playback!\n");
            snd_pcm_prepare(_aud_dev);
            return 0;
        } else if (ret < 0) {
            VLOGF_WARN("Can't write to PCM "
                    "device. %s\n",
                    snd_strerror(ret));
            return 0;
        }
    }

    return 1;
#endif /* USE_ALSA  */
}

/*
 *    Gets the playback bits per sample, sample rate, channels, and buffer size.
 *
 *    @param unsigned int *bits_per_samp    The bits per sample.
 *    @param unsigned int *sample_rate      The sample rate.
 *    @param unsigned int *num_channels     The channels.
 *    @param unsigned int *buf_len          The buffer size.
 */
void platform_get_sound_info(unsigned int *bits_per_samp,
                                        unsigned int *sample_rate,
                             unsigned int *num_channels, unsigned int *buf_len) {
    *bits_per_samp = PCM_SAMPLE_WIDTH;
    *sample_rate   = PCM_SAMPLE_RATE;
    *num_channels  = PCM_CHANNELS;
    *buf_len       = PCM_BUFFER_SIZE;
}

/*
 *    Reads from stdin.
 *
 *    @return char *    The string read from stdin.
 */
char *platform_read_stdin() {
#if __unix__
    static char buf[MAX_STDIN_READ] = {0};

    if (read(0, &buf, 1) > 0)
        return buf;

    return nullptr;
#elif _WIN32
#endif /* __unix__  */
}

/*
 *    Initializes the platform.
 *
 *    @return unsigned int    1 if successful, 0 otherwise.
 */
unsigned int platform_init(void) {
    if (!audio_init()) {
        LOGF_ERR("Unable to initialize audio.\n");
        return 0;
    }

    if (!surface_init()) {
        LOGF_ERR("Unable to initialize surface.\n");
        return 0;
    }

    input_parse("./aliases_sdl.txt");

#if __unix__
    fcntl(0, F_SETFL, O_NONBLOCK);
#endif /* __unix__  */

    return 1;
}

/*
 *    Updates the platform.
 *
 *    @param  float dt    Delta time.
 *
 *    @return unsigned int    1 if successful, 0 otherwise.
 */
unsigned int platform_update(float dt) {
    memset(_key_mask, 0, sizeof(_key_mask));
    input_capture();
    return 1;
}

/*
 *    Cleans up the platform.
 *
 *    @return unsigned int    1 if successful, 0 otherwise.
 */
unsigned int platform_cleanup(void) {
    audio_quit();
    surface_quit();
    return 1;
}

CHIK_MODULE(platform_init, platform_update, platform_cleanup)