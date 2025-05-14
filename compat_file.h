#ifndef __COMPAT_FILE_H__
#define __COMPAT_FILE_H__

#ifdef USE_TOSLIBC
#include <tos/gemdos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct { int handle; } FILE;

static inline const char *strchr(const char *s, int c)
{
    while (*s) {
        if (*s == c)
            return s;
        s++;
    }
    return NULL;
}

static inline FILE *fopen(const char *path, const char *mode)
{
    int h = -1;
    int read = 0, write = 0, create = 0, append = 0;

    switch (mode[0]) {
        case 'r':
            read = 1;
            if (strchr(mode, '+'))
                write = 1;
            break;
        case 'w':
            write = 1;
            create = 1;
            break;
        case 'a':
            write = 1;
            append = 1;
            break;
        default:
            return NULL;
    }

    if (create) {
        h = gemdos_fcreate(path, 0);
    } else {
        h = gemdos_fopen(path, read && write ? 0x2 : 0x0);
        if (h < 0)
            return NULL;
    }

    if (h < 0)
        return NULL;

    FILE *f = malloc(sizeof(FILE));
    if (!f) {
        gemdos_fclose(h);
        return NULL;
    }

    f->handle = h;

    if (append)
        gemdos_fseek(0, h, 2);  // Seek to end for append

    return f;
}

static inline void fclose(FILE *f)
{
    if (!f) return;

    gemdos_fclose(f->handle);
    free(f);
}

static inline size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *f)
{
    if (!f) return 0;

    return gemdos_fwrite(f->handle, size * nmemb, ptr) / size;
}

static inline size_t fread(void *ptr, size_t size, size_t nmemb, FILE *f)
{
    if (!f) return 0;

    return gemdos_fread(f->handle, size * nmemb, ptr) / size;
}

static inline long ftell(FILE *f)
{
    if (!f) return -1;

    return gemdos_fseek(0, f->handle, 1);
}

static inline int fseek(FILE *f, long offset, int whence)
{
    if (!f) return -1;

    gemdos_fseek(offset, f->handle, whence);
    return 0;
}

#else

#include <stdio.h>

#endif /* USE_TOSLIBC */

#endif /* __COMPAT_FILE_H__ */
