/*
 *    stat.c    --    source file for engine statistics
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 16, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    This file defines the routines for collecting and displaying
 *    engine statistics.
 */
#include "stat.h"

#include <sys/time.h>

stat_t gStat       = { 0 };

/*
 *    Starts a new frame.
 */
void stat_start_frame() {
    struct timeval tv;
    gettimeofday( &tv, NULL );

    /*
     *    Get the microseconds since the epoch.
     *
     *    The interesting array index increments the 
     *    frame count, and bounds it to not exceed
     *    the maximum number of averaging frames.
     */
    gStat.aFrameTimes[ gStat.aFrames++ % FRAMES_AVG_COUNT ] = tv.tv_sec * 1000000 + tv.tv_usec;

    /*
     *    Calculate the fps.
     */
    gStat.aFrameRate = 0;

    for ( u64 i = 0; i <= FRAMES_AVG_COUNT; i++ ) {
        /*
         *    Divide out the microsecond precision, and subtract from previous time.
         *    The use of the modulo operator is to prevent the frame rate from
         *    going negative.
         */
        gStat.aFrameRate += ( 
              gStat.aFrameTimes[ ( gStat.aFrames + i + 1  ) % FRAMES_AVG_COUNT ] 
            - gStat.aFrameTimes[ ( gStat.aFrames + i      ) % FRAMES_AVG_COUNT ] 
        ) / 100000.0f;
    }

    gStat.aFrameRate /= ( FRAMES_AVG_COUNT - 1 );
    gStat.aFrameRate  = 1.0f / gStat.aFrameRate;
}

/*
 *    Returns the engine statistics.
 *
 *    @return stat_t *    The engine statistics.
 */
stat_t *stat_get() {
    return &gStat;
}

/*
 *    Returns the frame rate.
 *
 *    @return f32    The frame rate.
 */
f32 stat_get_frame_rate() {
    return gStat.aFrameRate;
}