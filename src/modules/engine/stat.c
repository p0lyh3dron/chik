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
#include <math.h>

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
    gStat.aTimeDiff = gStat.aPrevTime;
    gStat.aPrevTime = tv.tv_sec * 1000000 + tv.tv_usec;
    gStat.aTimeDiff = gStat.aPrevTime - gStat.aTimeDiff;
    gStat.aFrameTimes[ gStat.aFrames++ % FRAMES_AVG_COUNT ] = gStat.aPrevTime;

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

    /*
     *    Calculate the average frame rate.
     */
    if ( !isinf( gStat.aFrameRate ) && !isnan( gStat.aFrameRate ) ) {
        gStat.aAvgFrameRate = ( gStat.aFrames * gStat.aAvgFrameRate + gStat.aFrameRate ) / ( gStat.aFrames + 1 );

        /*
         *    Calculate the maximum frame rate.
         */
        if ( gStat.aFrameRate > gStat.aMaxFrameRate ) {
            gStat.aMaxFrameRate = gStat.aFrameRate;
        }
    }
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
 *    Returns the difference between the current time and the
 *    previous time.
 * 
 *    @return s64    The difference between the current time and the
 *                   previous time in microseconds.
 */
s64 stat_get_time_diff() {
    return gStat.aTimeDiff;
}

/*
 *    Returns the frame rate.
 *
 *    @return f32    The frame rate.
 */
f32 stat_get_frame_rate() {
    return gStat.aFrameRate;
}

/*
 *    Returns the number of frames.
 *
 *    @return s64    The number of frames.
 */
s64 stat_get_frames() {
    return gStat.aFrames;
}

/*
 *    Returns the start time.
 *
 *    @return s64    The start time.
 */
s64 stat_get_start_time() {
    return gStat.aStartTime;
}

/*
 *    Dumps the engine statistics to a file.
 *
 *    @param const s8 *    The file name.
 * 
 *    @return u32          Returns 0 on failure, 1 on success.
 */
u32 stat_dump( const s8 *spFile ) {
    FILE *fp = fopen( spFile, "w" );
    if ( fp == nullptr ) {
        return 0;
    }

    fprintf( fp, "Frames: %lld\n", gStat.aFrames );
    fprintf( fp, "Frame rate: %f\n", gStat.aFrameRate );
    fprintf( fp, "Average frame rate: %f\n", gStat.aAvgFrameRate );
    fprintf( fp, "Maximum frame rate: %f\n", gStat.aMaxFrameRate );
    fprintf( fp, "Start time: %lld\n", gStat.aStartTime );

    fclose( fp );

    return 1;
}