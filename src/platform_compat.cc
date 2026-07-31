#include "platform_compat.h"

#include <filesystem>

#ifdef _WIN32
#include <io.h>
#include <stdlib.h>
#else
#include <dirent.h>
#endif

#include <SDL.h>

namespace fallout {

static void compat_prepare_native_path(char* nativePath, const char* path)
{
    strncpy(nativePath, path, COMPAT_MAX_PATH - 1);
    nativePath[COMPAT_MAX_PATH - 1] = '\0';
    compat_windows_path_to_native(nativePath);
    compat_resolve_path(nativePath);
}

int compat_stricmp(const char* string1, const char* string2)
{
    return SDL_strcasecmp(string1, string2);
}

int compat_strnicmp(const char* string1, const char* string2, size_t size)
{
    return SDL_strncasecmp(string1, string2, size);
}

char* compat_strupr(char* string)
{
    return SDL_strupr(string);
}

char* compat_strlwr(char* string)
{
    return SDL_strlwr(string);
}

char* compat_itoa(int value, char* buffer, int radix)
{
    return SDL_itoa(value, buffer, radix);
}

void compat_splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
    std::filesystem::path fsPath(path);

    if (drive != NULL) {
        strncpy(drive, fsPath.root_name().string().c_str(), COMPAT_MAX_DRIVE - 1);
    }

    if (dir != NULL) {
        strncpy(dir, fsPath.parent_path().string().c_str(), COMPAT_MAX_DIR - 1);
    }

    if (fname != NULL) {
        strncpy(fname, fsPath.stem().string().c_str(), COMPAT_MAX_FNAME - 1);
    }

    if (ext != NULL) {
        strncpy(ext, fsPath.extension().string().c_str(), COMPAT_MAX_EXT - 1);
    }
}

void compat_makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
{
    std::filesystem::path fsPath;

    if (drive != NULL && *drive != '\0') {
        fsPath /= drive;
    }

    if (dir != NULL && *dir != '\0') {
        fsPath /= dir;
    }

    if (fname != NULL && *fname != '\0') {
        fsPath /= fname;
    }

    if (ext != NULL && *ext != '\0') {
        fsPath += ext;
    }

    strncpy(path, fsPath.string().c_str(), COMPAT_MAX_PATH - 1);
}

int compat_read(int fileHandle, void* buf, unsigned int size)
{
    return read(fileHandle, buf, size);
}

int compat_write(int fileHandle, const void* buf, unsigned int size)
{
    return write(fileHandle, buf, size);
}

long compat_lseek(int fileHandle, long offset, int origin)
{
    return lseek(fileHandle, offset, origin);
}

long compat_tell(int fd)
{
    return lseek(fd, 0, SEEK_CUR);
}

long compat_filelength(const char* path)
{
    char nativePath[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativePath, path);
    return std::filesystem::file_size(nativePath);
}

int compat_mkdir(const char* path)
{
    std::error_code ec;
    char nativePath[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativePath, path);
    std::filesystem::create_directory(nativePath, ec);
    return ec.value();
}

unsigned int compat_timeGetTime()
{
    return SDL_GetTicks64();
}

FILE* compat_fopen(const char* path, const char* mode)
{
    char nativePath[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativePath, path);
    return fopen(nativePath, mode);
}

int compat_remove(const char* path)
{
    std::error_code err;
    char nativePath[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativePath, path);
    std::filesystem::remove(nativePath, err);
    return err.value();
}

int compat_rename(const char* oldFileName, const char* newFileName)
{
    std::error_code err;

    char nativeOldFileName[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativeOldFileName, oldFileName);

    char nativeNewFileName[COMPAT_MAX_PATH];
    compat_prepare_native_path(nativeNewFileName, newFileName);

    std::filesystem::rename(nativeOldFileName, nativeNewFileName, err);
    return err.value();
}

void compat_windows_path_to_native(char* path)
{
#ifndef _WIN32
    char* pch = path;
    while (*pch != '\0') {
        if (*pch == '\\') {
            *pch = '/';
        }
        pch++;
    }
#endif
}

void compat_resolve_path(char* path)
{
#ifndef _WIN32
    char* pch = path;

    DIR* dir;
    if (pch[0] == '/') {
        dir = opendir("/");
        pch++;
    } else {
        dir = opendir(".");
    }

    while (dir != NULL) {
        while (*pch == '/') {
            pch++;
        }

        if (*pch == '\0') {
            closedir(dir);
            break;
        }

        char* sep = strchr(pch, '/');
        size_t length;
        if (sep != NULL) {
            length = sep - pch;
        } else {
            length = strlen(pch);
        }

        bool found = false;

        struct dirent* entry = readdir(dir);
        while (entry != NULL) {
            if (strlen(entry->d_name) == length && compat_strnicmp(pch, entry->d_name, length) == 0) {
                strncpy(pch, entry->d_name, length);
                found = true;
                break;
            }
            entry = readdir(dir);
        }

        closedir(dir);
        dir = NULL;

        if (!found) {
            break;
        }

        if (sep == NULL) {
            break;
        }

        *sep = '\0';
        dir = opendir(path);
        *sep = '/';

        pch = sep + 1;
    }
#endif
}

char* compat_strdup(const char* string)
{
    return SDL_strdup(string);
}

// It's a replacement for compat_filelength(fileno(stream)) on platforms without
// fileno defined.
long getFileSize(FILE* stream)
{
    long originalOffset = ftell(stream);
    fseek(stream, 0, SEEK_END);
    long filesize = ftell(stream);
    fseek(stream, originalOffset, SEEK_SET);
    return filesize;
}

} // namespace fallout
