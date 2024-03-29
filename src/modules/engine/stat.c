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

#include <math.h>


#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif


stat_t _stat = {0};

/*
 *    Starts a new frame.
 */
void stat_start_frame() {
    size_t  i;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    /*
     *    Get the microseconds since the epoch.
     *
     *    The interesting array index increments the
     *    frame count, and bounds it to not exceed
     *    the maximum number of averaging frames.
     */
    _stat.time_diff                                      = _stat.prev_time;
    _stat.prev_time                                      = tv.tv_sec * 1000000 + tv.tv_usec;
    _stat.time_diff                                      = _stat.prev_time - _stat.time_diff;
    _stat.frame_times[_stat.frames++ % FRAMES_AVG_COUNT] = _stat.prev_time;

    /*
     *    Calculate the fps.
     */
    _stat.frame_rate = 0;

    for (i = 0; i <= FRAMES_AVG_COUNT; i++) {
        /*
         *    Divide out the microsecond precision, and subtract from previous
         * time. The use of the modulo operator is to prevent the frame rate
         * from going negative.
         */
        _stat.frame_rate +=
            (_stat.frame_times[(_stat.frames + i + 1) % FRAMES_AVG_COUNT] -
             _stat.frame_times[(_stat.frames + i) % FRAMES_AVG_COUNT]) /
            100000.0f;
    }

    _stat.frame_rate /= (FRAMES_AVG_COUNT - 1);
    _stat.frame_rate = 1.0f / _stat.frame_rate;

    /*
     *    Calculate the average frame rate.
     */
    if (!isinf(_stat.frame_rate) && !isnan(_stat.frame_rate)) {
        _stat.frame_rate_avg =
            (_stat.frames * _stat.frame_rate_avg + _stat.frame_rate) /
            (_stat.frames + 1);

        /*
         *    Calculate the maximum frame rate.
         */
        if (_stat.frame_rate > _stat.frame_rate_max) {
            _stat.frame_rate_max = _stat.frame_rate;
        }
    }

    printf("Frame rate: %f\n", _stat.frame_rate);
}

/*
 *    Returns the engine statistics.
 *
 *    @return stat_t *    The engine statistics.
 */
stat_t *stat_get() { return &_stat; }

/*
 *    Returns the difference between the current time and the
 *    previous time.
 *
 *    @return long    The difference between the current time and the
 *                   previous time in microseconds.
 */
long stat_get_time_diff() { return _stat.time_diff; }

/*
 *    Returns the frame rate.
 *
 *    @return float    The frame rate.
 */
float stat_get_frame_rate() { return _stat.frame_rate; }

/*
 *    Returns the number of frames.
 *
 *    @return long    The number of frames.
 */
long stat_get_frames() { return _stat.frames; }

/*
 *    Returns the start time.
 *
 *    @return long    The start time.
 */
long stat_get_start_time() { return _stat.start_time; }

/*
 *    Dumps the engine statistics to a file.
 *
 *    @param const char *file    The file name.
 *
 *    @return unsigned int          Returns 0 on failure, 1 on success.
 */
unsigned int stat_dump(const char *file) {
    FILE *fp = fopen(file, "w");
    if (fp == nullptr) {
        return 0;
    }

    fprintf(fp, "Frames: %lld\n", _stat.frames);
    fprintf(fp, "Frame rate: %f\n", _stat.frame_rate);
    fprintf(fp, "Average frame rate: %f\n", _stat.frame_rate_avg);
    fprintf(fp, "Maximum frame rate: %f\n", _stat.frame_rate_max);
    fprintf(fp, "Start time: %lld\n", _stat.start_time);

    fclose(fp);

    return 1;
}