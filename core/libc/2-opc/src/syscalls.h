/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#ifndef SYSCALLS_H_INCLUDED
#define SYSCALLS_H_INCLUDED

_Noreturn void __sys_halt(int exit_code);
int __sys_time(unsigned out_buffer[3]);
int __sys_spawn(void /*TODO*/);
int __sys_fopen(const char* path, bool writeable);
int __sys_fclose(int handle);
int __sys_fread(int handle, void* out_buffer, unsigned size);
int __sys_fwrite(int handle, const void* buffer, unsigned size);
int __sys_fseek(int handle, unsigned position_low, unsigned position_high);
int __sys_ftell(int handle, unsigned out_position[2]);
int __sys_ftrunc(int handle, unsigned position_low, unsigned position_high);
int __sys_stat(const char* path, unsigned out_stat[4]);
int __sys_rename(const char* from, const char* to);
int __sys_symlink(const char* from, const char* to);
int __sys_unlink(const char* path);
int __sys_chmod(const char* path, int mode);
int __sys_mkdir(const char* path);
int __sys_rmdir(const char* path);
int __sys_dopen(const char* path);
int __sys_dclose(int handle);
int __sys_dread(int handle, char out_buffer[256]);

#endif
