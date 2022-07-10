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
    f32 aAvgFrameRate;
    f32 aMaxFrameRate;
    s64 aStartTime;
    s64 aPrevTime;
    s64 aTimeDiff;
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
 *    Returns the difference between the current time and the
 *    previous time.
 * 
 *    @return s64    The difference between the current time and the
 *                   previous time in microseconds.
 */
s64 stat_get_time_diff();

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

/*
 *    Dumps the engine statistics to a file.
 *
 *    @param const s8 *    The file name.
 * 
 *    @return u32          Returns 0 on failure, 1 on success.
 */
u32 stat_dump( const s8 *spFile );