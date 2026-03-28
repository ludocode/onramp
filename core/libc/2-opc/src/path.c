/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <__onramp/__fatal.h>
#include <__onramp/__pit.h>

static char* realpath_impl(const char* restrict path, char* restrict buffer,
        const char* restrict workdir);

/*
 * This stores the modified and normalized working directory.
 *
 * The Onramp libc performs file syscalls using absolute paths. Whenever a
 * relative path is passed to a libc function, we prepend the working directory
 * before making the syscall.
 *
 * The first time the working directory is requested or changed, we normalize
 * it and store it here.
 *
 * When the program changes the working directory via chdir(), we don't inform
 * the OS or VM in any way. We check it for validity and then just store it. It
 * is then used as the new working directory for future syscalls.
 */
static char* current_workdir;

/*
 * Returns the internal string containing the working directory, initializing
 * it if necessary.
 *
 * This function should be used to get the working directory throughout the
 * libc.
 */
char* __onramp_workdir(void) {
    if (current_workdir) {
        return current_workdir;
    }

    const char* pit_wd = (const char*)__process_info_table[__ONRAMP_PIT_WORKDIR];
    if (pit_wd == 0 || *pit_wd == 0) {
        // No working directory in the PIT. We may pass relative paths to
        // VM syscalls.
        current_workdir = strdup("");
    } else {
        current_workdir = realpath_impl(pit_wd, NULL, "");

        // If we failed to normalize the working directory, we just use it
        // as is.
        // TODO print a warning or something
        if (current_workdir == NULL) {
            current_workdir = strdup(pit_wd);
        }
    }

    if (current_workdir == NULL) {
        __fatal("Out of memory.");
    }

    return current_workdir;
}

int chdir(const char* path) {

    // The given path might be relative to the current path. Make it absolute.
    char* new_workdir = realpath(path, NULL);
    if (new_workdir == NULL) {
        // If realpath() failed it will have set errno for us.
        return -1;
    }

    // TODO we should try to diropen the path to make sure it exists, is a
    // directory, and is readable. or at least stat it and ensure it is a
    // directory.

    free(current_workdir);
    current_workdir = new_workdir;
    return 0;
}

char* getcwd(char* out_buffer, size_t size) {
    char* wd = __onramp_workdir();
    size_t len = strlen(wd);

    if (size != 0 && len > size - 1) {
        errno = ERANGE;
        return NULL;
    }

    if (out_buffer) {
        memcpy(out_buffer, wd, len + 1);
        return out_buffer;
    }

    // NULL out_buffer is a GNU extension. According to the docs, unless size
    // is zero, the allocated buffer must have length size; we can't truncate
    // it to the proper length.
    char* ret = malloc((size == 0) ? len + 1 : size);
    if (ret == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    memcpy(ret, wd, len + 1);
    return ret;
}

char* get_current_dir_name(void) {
    return getcwd(NULL, 0);
}

char* getwd(char out_buffer[PATH_MAX]) {
    if (out_buffer == NULL) {
        errno = EINVAL;
        return NULL;
    }

    char* ret = getcwd(out_buffer, PATH_MAX);

    if (ret == NULL && errno == ERANGE) {
        // getwd() returns ENAMETOOLONG instead of ERANGE.
        errno = ENAMETOOLONG;
    }
    return ret;
}

/*
 * The implementation of realpath().
 *
 * This is a bit complicated because unlike a real realpath(), we have to
 * support the case where the VM doesn't give us a working directory (or even
 * gives us a relative working directory), in which case all relative paths
 * have to stay relative.
 *
 * To make this work, each component is appending with a trailing '/', this way
 * it works whether or not the overall path is relative. We trim the final '/'
 * at the end.
 */
static char* realpath_impl(const char* restrict path, char* restrict buffer,
        const char* /*nullable*/ restrict workdir)
{
    if (path == 0) {
        errno = EINVAL;
        return NULL;
    }
    if (path[0] == 0) {
        errno = ENOENT;
        return NULL;
    }

    char* allocated = NULL;
    if (buffer == NULL) {
        buffer = (allocated = malloc(PATH_MAX));
        if (buffer == NULL) {
            errno = ENOMEM;
            return NULL;
        }
    }

    char* buffer_pos = buffer;
    char* buffer_end = buffer + PATH_MAX;

    if (path[0] != '/') {
        // Append working directory. It has already been normalized (and we
        // know it fits in PATH_MAX) so we don't need to check it.
        size_t workdir_len = strlen(workdir);
        if (workdir_len > 0) {
            memcpy(buffer_pos, workdir, workdir_len);
            buffer_pos[workdir_len] = '/';
            buffer_pos += workdir_len + 1;
        }
    } else {
        // Append the leading "/" to the buffer
        buffer_pos[0] = '/';
        ++buffer_pos;
        ++path;
    }

    // Append each component to the buffer, resolving ".", "..", and symlinks.
    while (*path) {

        // skip redundant or trailing '/'.
        while (*path == '/')
            ++path;

        // seek to the end of the component
        const char* component = path;
        const char* component_end = path;
        while (*component_end != '/' && *component_end != 0)
            ++component_end;

        // "." component is redundant
        if (component_end == path + 1 && *path == '.') {
            path += 1;
            continue;
        }

        // ".." backs up by one component.
        if (component_end == path + 2 && path[0] == '.' && path[1] == '.') {
            path += 2;

            if (buffer_pos == buffer + 1 && buffer[0] == '/') {
                // We're already at the root. The root's ".." is itself.
                continue;
            }

            if (buffer_pos == buffer) {
                // If we don't have any path component, it can only be because
                // the VM passed a relative path or nothing as the working
                // directory. We don't allow ".." to escape the relative
                // working directory (we never pass ".." to the VM) so we have
                // to fail.
                errno = EACCES;
                goto fail;
            }

            // Remove the last path component
            do {
                --buffer_pos;
            } while (buffer_pos != buffer && buffer_pos[-1] != '/');
            continue;
        }

        // Otherwise we append the component
        uintptr_t component_len = component_end - component;
        if (component_len + 1 > buffer_end - buffer_pos) {
            errno = ENAMETOOLONG;
            goto fail;
        }
        memcpy(buffer_pos, component, component_len);
        buffer_pos[component_len] = '/';
        buffer_pos += component_len + 1;

        if (*component_end == 0)
            break;
        path = component_end + 1;

        // TODO check if it's a symlink; if it is, we need to recurse this
        // component append logic. We don't have symlink support in the VM yet.
    }

    // Remove the trailing '/'
    if (buffer_pos[-1] == '/' && buffer_pos - buffer != 1) {
        --buffer_pos;
    }

    // Add the null-terminator
    if (buffer_pos == buffer_end) {
        errno = ENAMETOOLONG;
        goto fail;
    }
    *buffer_pos++ = 0;

    // Shrink the allocation down to the appropriate size
    if (allocated) {
        char* shrunk = realloc(allocated, buffer_pos - buffer);
        if (!shrunk) {
            // it should not be possible for a shrinking realloc() to fail
            __fatal("realloc() failed to shrink");
        }
        return shrunk;
    }

    return buffer;

fail:
    if (allocated) {
        free(allocated);
    }
    return NULL;
}

char* realpath(const char* restrict path, char* restrict buffer) {
    return realpath_impl(path, buffer, __onramp_workdir());
}
