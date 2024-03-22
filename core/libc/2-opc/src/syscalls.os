; The MIT License (MIT)
;
; Copyright (c) 2023-2024 Fraser Heavy Software
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.



; This contains wrapper functions for Onramp system calls so we can implement
; as much as possible in C.



; ==========================================================
; [[noreturn]] void __sys_halt(int exit_code);
; ==========================================================

=__sys_halt
    sys halt '00 '00



; ==========================================================
; int __sys_time(unsigned buffer[3]);
; ==========================================================

=__sys_time
    sys time '00 '00



; ==========================================================
; int __sys_spawn(TODO);
; ==========================================================

=__sys_spawn
    sys spawn '00 '00



; ==========================================================
; int __sys_fopen(const char* path, bool writeable);
; ==========================================================

=__sys_fopen
    sys fopen '00 '00



; ==========================================================
; int __sys_fclose(int handle);
; ==========================================================

=__sys_fclose
    sys fclose '00 '00



; ==========================================================
; int __sys_fread(int handle, void* out_buffer, unsigned size);
; ==========================================================

=__sys_fread
    sys fread '00 '00



; ==========================================================
; void __sys_fwrite(int handle, const void* buffer, unsigned size);
; ==========================================================

=__sys_fwrite
    sys fwrite '00 '00



; ==========================================================
; void __sys_fseek(int handle, unsigned position_low, unsigned position_high);
; ==========================================================

=__sys_fseek
    sys fseek '00 '00



; ==========================================================
; void __sys_fseek(int handle, unsigned out_position[3]);
; ==========================================================

=__sys_fseek
    sys fseek '00 '00



; ==========================================================
; void __sys_ftrunc(int handle, unsigned position_low, unsigned position_high);
; ==========================================================

=__sys_ftrunc
    sys ftrunc '00 '00



; ==========================================================
; void __sys_stat(const char* path, unsigned out_stat[4]);
; ==========================================================

=__sys_stat
    sys stat '00 '00



; ==========================================================
; void __sys_rename(const char* from, const char* to);
; ==========================================================

=__sys_rename
    sys rename '00 '00



; ==========================================================
; void __sys_symlink(const char* from, const char* to);
; ==========================================================

=__sys_symlink
    sys symlink '00 '00



; ==========================================================
; void __sys_unlink(const char* path);
; ==========================================================

=__sys_unlink
    sys unlink '00 '00



; ==========================================================
; void __sys_chmod(const char* path, int mode);
; ==========================================================

=__sys_chmod
    sys chmod '00 '00



; ==========================================================
; void __sys_mkdir(const char* path);
; ==========================================================

=__sys_mkdir
    sys mkdir '00 '00



; ==========================================================
; void __sys_rmdir(const char* path);
; ==========================================================

=__sys_rmdir
    sys rmdir '00 '00



; ==========================================================
; int __sys_dopen(const char* path);
; ==========================================================

=__sys_dopen
    sys dopen '00 '00



; ==========================================================
; void __sys_dclose(int handle);
; ==========================================================

=__sys_dclose
    sys dclose '00 '00



; ==========================================================
; int __sys_dread(int handle, char out_buffer[256]);
; ==========================================================

=__sys_dread
    sys dread '00 '00
