/*
 * AllAlike - the POSIX calls MinGW lacks, force-included into every Windows compile by
 * tools/do-windows-plugin after G2Alike's winCompat.h, so the shared sources need no #ifdefs for them.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __ALIKE_COMPAT_H__
#define __ALIKE_COMPAT_H__

#if defined (_WIN32) && !defined (__cplusplus)

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// macOS names only the calling thread; winpthreads takes the thread as well
#define pthread_setname_np(name) pthread_setname_np(pthread_self(), (name))

// One writer per file at a time here (the preset bank), so seek-then-write is as good as pwrite
static inline ssize_t pwrite(int fd, const void * buf, size_t count, off_t offset) {
    if (lseek(fd, offset, SEEK_SET) != offset) {
        return -1;
    }
    return (ssize_t)write(fd, buf, (unsigned int)count);
}

static inline char * strcasestr(const char * haystack, const char * needle) {
    size_t n = strlen(needle);

    for ( ; *haystack != '\0'; haystack++) {
        if (strncasecmp(haystack, needle, n) == 0) {
            return (char *)haystack;
        }
    }
    return (n == 0) ? (char *)haystack : NULL;
}

// Windows' stat() reports owner 0 for every file, so the backdoor's "is it mine" check is 0 == 0
static inline int getuid(void) {
    return 0;
}

// HOME is not set on Windows, and the presets and sounds paths start from it: alikeWin.c answers
// %USERPROFILE% for it, with forward slashes, since the folder code splits paths on '/'. Ours only - the
// host's environment is left alone.
char * alike_getenv(const char * name);
#define getenv(name)    alike_getenv(name)

// The panel font: Arial from the Windows fonts folder, where the Mac reads it from /System/Library/Fonts.
const char * alike_font_path(void);

#endif // _WIN32 && !__cplusplus

#endif // __ALIKE_COMPAT_H__
