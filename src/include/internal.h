//
// Created by eric on 8/20/19.
//

#ifndef OPEN_RPG_INTERNAL_H
#define OPEN_RPG_INTERNAL_H 1

#include "rpg.h"
#include "platform.h"

#define RPG_ENSURE_FILE(filename) if (!RPG_FILE_EXISTS(filename)) return RPG_ERR_FILE_NOT_FOUND

#define RPG_RETURN_IF_NULL(ptr) if (ptr == NULL) return RPG_ERR_NULL_POINTER

static inline int maxi(int i1, int i2) {
    return i1 > i2 ? i1 : i2;
}

static inline int mini(int i1, int i2) {
    return i1 < i2 ? i1 : i2;
}

#define RPG_CLAMPF(v,min,max) fmaxf(min, fminf(max, v))

#define RPG_CLAMPI(v,min,max) maxi(min, mini(v, max))

#endif /* OPEN_RPG_INTERNAL_H */
