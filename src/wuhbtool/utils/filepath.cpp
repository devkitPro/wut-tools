#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "types.h"
#include "filepath.h"

void os_strncpy(oschar_t *dst, const char *src, size_t size) {
#ifdef _WIN32
    MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, size);
#else
    strncpy(dst, src, size);
#endif
}

void os_strncpy_to_char(char *dst, const oschar_t *src, size_t size) {
#ifdef _WIN32
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, size, NULL, NULL);
#else
    strncpy(dst, src, size);
#endif
}

int os_makedir(const oschar_t *dir) {
#ifdef _WIN32
    return _wmkdir(dir);
#else
    return mkdir(dir, 0777);
#endif
}

int os_rmdir(const oschar_t *dir) {
#ifdef _WIN32
    return _wrmdir(dir);
#else
    return remove(dir);
#endif
}

void filepath_update(filepath_t *fpath) {
    memset(fpath->os_path, 0, MAX_OSPATH * sizeof(oschar_t));
    os_strncpy(fpath->os_path, fpath->char_path, MAX_OSPATH);
}

void filepath_init(filepath_t *fpath) {
    fpath->valid = VALIDITY_INVALID;
}

void filepath_copy(filepath_t *fpath, filepath_t *copy) {
    if (copy != NULL && copy->valid == VALIDITY_VALID)
        memcpy(fpath, copy, sizeof(filepath_t));
    else
        memset(fpath, 0, sizeof(filepath_t));
}

void filepath_os_append(filepath_t *fpath, oschar_t *path) {
    char tmppath[MAX_OSPATH];
    if (fpath->valid == VALIDITY_INVALID)
        return;

    memset(tmppath, 0, MAX_OSPATH);

    os_strncpy_to_char(tmppath, path, MAX_OSPATH);
    strcat(fpath->char_path, OS_PATH_SEPARATOR);
    strcat(fpath->char_path, tmppath);
    filepath_update(fpath);
}

void filepath_append(filepath_t *fpath, const char *format, ...) {
    char tmppath[MAX_OSPATH];
    va_list args;

    if (fpath->valid == VALIDITY_INVALID)
        return;

    memset(tmppath, 0, MAX_OSPATH);

    va_start(args, format);
    vsnprintf(tmppath, sizeof(tmppath), format, args);
    va_end(args);

    strcat(fpath->char_path, OS_PATH_SEPARATOR);
    strcat(fpath->char_path, tmppath);
    filepath_update(fpath);
}

void filepath_append_n(filepath_t *fpath, uint32_t n, const char *format, ...) {
    char tmppath[MAX_OSPATH];
    va_list args;

    if (fpath->valid == VALIDITY_INVALID || n > MAX_OSPATH)
        return;

    memset(tmppath, 0, MAX_OSPATH);

    va_start(args, format);
    vsnprintf(tmppath, sizeof(tmppath), format, args);
    va_end(args);

    strcat(fpath->char_path, OS_PATH_SEPARATOR);
    strncat(fpath->char_path, tmppath, n);
    filepath_update(fpath);
}

void filepath_set(filepath_t *fpath, const char *path) {
    if (strlen(path) < MAX_OSPATH) {
        fpath->valid = VALIDITY_VALID;
        memset(fpath->char_path, 0, MAX_OSPATH);
        strncpy(fpath->char_path, path, MAX_OSPATH);
        filepath_update(fpath);
    } else {
        fpath->valid = VALIDITY_INVALID;
    }
}

void filepath_os_set(filepath_t *fpath, oschar_t *path) {
    char tmppath[MAX_OSPATH];
    memset(tmppath, 0, MAX_OSPATH);
    os_strncpy_to_char(tmppath, path, MAX_OSPATH);

    filepath_set(fpath, tmppath);
}

oschar_t *filepath_get(filepath_t *fpath) {
    if (fpath->valid == VALIDITY_INVALID)
        return NULL;
    else
        return fpath->os_path;
}
