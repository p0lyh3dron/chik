/*
 *    stat.h    --    header file for engine statistics
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 16, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    The following declares the types and functions used
 *    for taking engine statistics, such as the number of
 *    frames rendered, the start time, etc.
 */
#pragma once

#define FRAMES_AVG_COUNT 10

#include "libchik.h"

typedef struct {
    s64 aFrames;
    s64 aFrameTimes[ FRAMES_AVG_COUNT ];
    f32 aFrameRate;
    s64 aStartTime;
} stat_t;

/*
 *    Starts a new frame.
 */
void stat_start_frame();

/*
 *    Returns the engine statistics.
 *
 *    @return stat_t *    The engine statistics.
 */
stat_t *stat_get();

/*
 *    Returns the frame rate.
 *
 *    @return f32    The frame rate.
 */
f32 stat_get_frame_rate();

/*
 *    Returns the number of frames.
 *
 *    @return s64    The number of frames.
 */
s64 stat_get_frames();

/*
 *    Returns the start time.
 *
 *    @return s64    The start time.
 */
s64 stat_get_start_time();