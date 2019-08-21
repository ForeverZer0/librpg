//
// Created by eric on 8/20/19.
//

#ifndef OPEN_RPG_INTERNAL_H
#define OPEN_RPG_INTERNAL_H 1

#include "rpg.h"

#if defined(RPG_WINDOWS)
#include <io.h>
#include <windows.h>
#define RPG_FILE_EXISTS(filename) (_access(filename, 0) == -1)
#defien RPG_SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define RPG_FILE_EXISTS(filename) (access(filename, 0) == -1)
#define RPG_SLEEP(ms) usleep(ms * 1000)
#endif

#define RPG_ENSURE_FILE(filename) if (RPG_FILE_EXISTS(filename)) return RPG_ERR_FILE_NOT_FOUND

#define RPG_RETURN_IF_NULL(ptr) if (ptr == NULL) return RPG_ERR_NULL_POINTER

#define RPG_CLAMPF(v,min,max) fmaxf(min, fminf(max, v))

#endif /* OPEN_RPG_INTERNAL_H */
