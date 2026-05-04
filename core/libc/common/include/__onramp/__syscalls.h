/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2026 Fraser Heavy Software
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

#ifndef __ONRAMP_LIBC_ONRAMP_SYSCALLS_H_INCLUDED
#define __ONRAMP_LIBC_ONRAMP_SYSCALLS_H_INCLUDED

#include <stdbool.h>

// System call numbers
#define __SYS_EXIT 0
#define __SYS_PANIC 1
#define __SYS_TIME 2
#define __SYS_OPEN 3
#define __SYS_CLOSE 4
#define __SYS_READ 5
#define __SYS_WRITE 6
#define __SYS_SEEK 7
#define __SYS_SIZE 8
#define __SYS_TRUNC 9
#define __SYS_DIRENT 12
#define __SYS_STAT 13
#define __SYS_RENAME 14
#define __SYS_DELETE 16
#define __SYS_CHMOD 17
#define __SYS_MKDIR 18
#define __SYS_DEBUG 22
#define __SYS_ALLOC 23
#define __SYS_FREE 24

// These were syscalls in v3. They are kept for backwards compatibility.
#define __SYS_FOPEN __SYS_OPEN
#define __SYS_FCLOSE __SYS_CLOSE
#define __SYS_FREAD __SYS_READ
#define __SYS_FWRITE __SYS_WRITE
#define __SYS_FSEEK __SYS_SEEK
#define __SYS_FTELL 8
#define __SYS_FTRUNC __SYS_TRUNC
#define __SYS_DOPEN 10
#define __SYS_DCLOSE 11
#define __SYS_DREAD __SYS_DIRENT
#define __SYS_UNLINK __SYS_DELETE
#define __SYS_RMDIR 19

// System call error codes
#define __ERROR_GENERIC (-1)
#define __ERROR_NO_SUCH_PATH (-2)
#define __ERROR_IO (-3)
#define __ERROR_UNSUPPORTED (-4)
#define __ERROR_TRY_LATER (-5)
#define __ERROR_END_OF_FILE (-6)
#define __ERROR_OVERFLOW (-7)
#define __ERROR_IN_USE (-8)

/**
 * Returns true if the given syscall is supported by the environment (the VM or
 * a parent program), false otherwise.
 */
bool __syscall_is_supported(int __syscall_number);

// The below functions directly call syscalls. They do NOT check whether the
// syscall is actually supported. Calling an unsupported syscall will jump to
// address 0 which will surely crash or worse. Use __syscall_is_supported()
// first before calling any optional syscalls.

_Noreturn void __sys_exit(int __exit_code);
_Noreturn void __sys_panic(int __exit_code);
int __sys_time(unsigned out_buffer[3]);
int __sys_open(const char* path, bool writeable);
int __sys_close(unsigned handle);
int __sys_read(unsigned handle, void* out_buffer, unsigned size);
int __sys_write(unsigned handle, const void* buffer, unsigned size);
int __sys_seek(unsigned handle, unsigned offset_low, unsigned offset_high); // does not work on v3 or earlier!
int __sys_size(unsigned handle, unsigned out_position[2]);
int __sys_trunc(unsigned handle, unsigned position_low, unsigned position_high);
int __sys_size(unsigned handle, unsigned out_position[2]); // does not work on v3 or earlier!
int __sys_dirent(unsigned handle, char out_buffer[256]);
int __sys_stat(const char* path, unsigned* out_size);
int __sys_rename(const char* from, const char* to);
int __sys_delete(const char* path);
int __sys_chmod(const char* path, unsigned mode);
int __sys_mkdir(const char* path);
/*int __sys_debug(...);*/
/*int __sys_alloc(...);*/
/*int __sys_free(...);*/

// These were syscalls in v3. They are kept for backwards compatibility.
#define __sys_fopen __sys_open
#define __sys_fclose __sys_close
#define __sys_fread __sys_read
#define __sys_fwrite __sys_write
int __sys_fseek(unsigned handle, unsigned base, unsigned offset_low, unsigned offset_high); // does not work on v4 or later!
#define __sys_ftell __sys_size  // same arguments and syscall number. does not work on v4 or later!
#define __sys_ftrunc __sys_trunc
int __sys_dopen(const char* path);
int __sys_dclose(int handle);
#define __sys_dread __sys_dirent
#define __sys_unlink __sys_delete
int __sys_rmdir(const char* path);

#endif
