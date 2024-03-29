/*
 *    engine.c    --    source for engine module
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 16, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    The brain of Chik Engine resides in this module. This
 *    file will define how the engine will do module handling.
 */
#include "engine.h"

#include <memory.h>
#include <stdarg.h>
#include <time.h>

#include "stat.h"

#ifdef _WIN32
#include <WinSock2.h>

#define WIN32_LEAN_AND_MEAN
#include <stdint.h>  // portable: uint64_t   MSVC: __int64

// https://stackoverflow.com/questions/10905892/equivalent-of-gettimeofday-for-windows
int gettimeofday(struct timeval *tp, struct timezone *tzp) {
  // Note: some broken versions only have 8 trailing zero's, the correct epoch
  // has 9 trailing zero's This magic number is the number of 100 nanosecond
  // intervals since January 1, 1601 (UTC) until 00:00:00 January 1, 1970
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

  SYSTEMTIME system_time;
  FILETIME file_time;
  uint64_t time;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec = (long)((time - EPOCH) / 10000000L);
  tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
  return 0;
}
#endif


#define CHIK_ENGINE_SHELL_MAX_COMMAND_LENGTH 256

module_t _modules[ENGINE_MAX_MODULES] = {0};

char *(*plat_read_stdin)() = nullptr;

char _shl_cmds[CHIK_ENGINE_SHELL_MAX_COMMAND_LENGTH] = {0};
int  _shl_cmd_indx                                   = 0;

/*
 *    Loads a function from the engine for external use.
 *
 *    @param const char *name    The name of the function to load.
 *
 *    @return void *       Returns a pointer to the function.
 */
void *engine_load_function(const char *name) {
    size_t i;
    void         *fun;

    for (i = 0; i < ENGINE_MAX_MODULES; i++) {
        /*
         *    If the module isn't null, we'll try to load the function.
         */
        if (_modules[i].name && _modules[i].handle) {
            fun = dl_sym(_modules[i].handle, name);

            if (fun)
                break;
        }
    }

    return fun;
}

/*
 *    Initializes the engine with the specified modules.
 *
 *    @param const char *modules    The name of the modules to initialize.
 *    @param ...                  The other modules to initialize.
 *
 *    @return unsigned int          Returns 0 on failure, 1 on success.
 */
unsigned int engine_init(const char *modules, ...) {
    va_list      args;
    dl_handle_t  h;
    unsigned int result      = 1;
    unsigned int module_indx = 0;
    stat_t      *stat        = stat_get();
    unsigned int (*entry)(void *);
    unsigned int (*update)(float);
    unsigned int (*exit)(void);

    /*
     *    Set the start time.
     */
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    stat->start_time = tv.tv_sec * 1000000.0 + tv.tv_usec;
    stat->prev_time  = stat->start_time;

    /*
     *    Initialize the engine modules.
     */
    va_start(args, modules);

    while (modules) {
        h = dl_open(modules);

        if (h == nullptr) {
            VLOGF_FAT("Unable to open module: %s: %s\n", modules, dl_error());
            result = 0;
            break;
        }

        entry  = dl_sym(h, "chik_module_entry");
        update = dl_sym(h, "chik_module_update");
        exit   = dl_sym(h, "chik_module_exit");

        if (entry != nullptr) {
            if (!entry(&engine_load_function)) {
                VLOGF_FAT("Unable to initialize module: %s\n", modules);
                result = 0;
                break;
            }
        } else {
            VLOGF_WARN("Unable to load module entry: %s\n", modules);
        }

        if (update == nullptr)
            VLOGF_WARN("Unable to load module update: %s\n", modules);

        if (exit == nullptr)
            VLOGF_WARN("Unable to load module exit: %s\n", modules);

        VLOGF_NOTE("Module loaded: %s\n", modules);

        _modules[module_indx++] = (module_t){h, modules, update, exit};

        modules = va_arg(args, const char *);
    }

    va_end(args);

    *(void **)(&plat_read_stdin) = engine_load_function("platform_read_stdin");

    if (!plat_read_stdin) {
        LOGF_FAT("Unable to load function platform_read_stdin\n");
        result = 0;
    }

    return result;
}

/*
 *    Updates the shell.
 *
 *    @return unsigned int           Returns 0 on failure, 1 on success.
 */
unsigned int engine_update_shell(void) {
    char *pBuf = plat_read_stdin();

#ifdef SHEEL
    if (pBuf != nullptr)
        memcpy(_shl_cmds + _shl_cmd_indx++, pBuf, 1);

    if (_shl_cmds[_shl_cmd_indx - 1] == '\n') {
        _shl_cmds[_shl_cmd_indx - 1] = '\0';
        shell_execute(_shl_cmds);
        _shl_cmd_indx = 0;
    }
#endif

    return 1;
}

/*
 *    Updates the engine.
 *
 *    @return unsigned int           Returns 0 on failure, 1 on success.
 */
unsigned int engine_update(void) {
    unsigned int i;
    unsigned int result = 1;
    float        dt     = (float)stat_get_time_diff() / 1000000.0f;

    stat_start_frame();
    engine_update_shell();

    for (i = 0; i < ENGINE_MAX_MODULES; i++)
        if (_modules[i].update != nullptr)
            result &= _modules[i].update(dt);

    return result;
}

/*
 *    Frees the engine.
 */
void engine_free() {
    long i;

    for (i = ENGINE_MAX_MODULES - 1; i >= 0; --i) {
        if (_modules[i].handle) {
            if (_modules[i].exit != nullptr) {
                if (_modules[i].exit()) {
                    VLOGF_NOTE("Module exited: %s\n", _modules[i].name);
                } else {
                    VLOGF_FAT("Module failed to exit: %s\n", _modules[i].name);
                }
            }
            dl_close(_modules[i].handle);
        }
    }

    if (!stat_dump("stats.txt"))
        LOGF_ERR("unsigned int engine_free(): Unable to dump stats\n");
}