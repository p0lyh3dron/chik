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
    long  frames;
    long  frame_times[FRAMES_AVG_COUNT];
    float frame_rate;
    float frame_rate_avg;
    float frame_rate_max;
    long  start_time;
    long  prev_time;
    long  time_diff;
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
 *    @return long    The difference between the current time and the
 *                   previous time in microseconds.
 */
long stat_get_time_diff();

/*
 *    Returns the frame rate.
 *
 *    @return float    The frame rate.
 */
float stat_get_frame_rate();

/*
 *    Returns the number of frames.
 *
 *    @return long    The number of frames.
 */
long stat_get_frames();

/*
 *    Returns the start time.
 *
 *    @return long    The start time.
 */
long stat_get_start_time();

/*
 *    Dumps the engine statistics to a file.
 *
 *    @param const char *file    The file name.
 *
 *    @return unsigned int          Returns 0 on failure, 1 on success.
 */
unsigned int stat_dump(const char *file);