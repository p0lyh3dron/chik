/*
 *    platform.h    --    header for platform specific stuff
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    Different platforms will have different nuances
 *    for presenting graphics and audio, as well as
 *    capturing input. This file will contain the
 *    abstracted interface for those platforms.
 *    For now, we will implement SDL for display and input,
 *    and ALSA for audio.
 */
#ifndef CHIK_PLATFORM_H
#define CHIK_PLATFORM_H

#include "libchik.h"

/*
 *    Draws a bitmap to the screen.
 *
 *    @param image_t *    The image to draw.
 *
 *    @return u32         1 if successful, 0 otherwise.
 */
u32 platform_draw_image( image_t *spImage );

/*
 *    Returns the width and height of the screen.
 *
 *    @return vec2u_t      The width and height of the screen.
 */
vec2u_t platform_get_screen_size( void );

/*
 *    Pops an event from the input queue.
 *
 *    @return s8 *    The event, or nullptr if there are no events.
 */
s8 *platform_get_event();

/*
 *    Returns a joystick event.
 *
 *    @return vec2u_t      The joystick event.
 */
vec2u_t platform_get_joystick_event();

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
u32 platform_write_sound( s8 *spData );

/*
 *    Gets the playback bits per sample, sample rate, channels, and buffer size.
 *
 *    @param u32 *    The bits per sample.
 *    @param u32 *    The sample rate.
 *    @param u32 *    The channels.
 *    @param u32 *    The buffer size.
 */
void platform_get_sound_info( u32 *spBitsPerSample, u32 *spSampleRate, u32 *spChannels, u32 *spBufferSize );

/*
 *    Reads from stdin.
 *
 *    @return s8 *    The string read from stdin.
 */
s8 *platform_read_stdin();

#endif /* CHIK_PLATFORM_H  */