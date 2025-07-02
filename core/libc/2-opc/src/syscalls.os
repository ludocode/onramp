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
; bool __syscall_is_supported(int syscall_number);
; ==========================================================
; Returns true if the given syscall is supported; false otherwise.
; ==========================================================

=__syscall_is_supported

    ; get the syscall table
    imw r8 ^__process_info_table
    ldw r8 rpp r8     ; r8 = process_info_table
    ldw r8 r8 8       ; r8 = syscall table

    ; load the syscall's address
    shl r7 r0 3       ; r7 == offset of syscall (r0 << 3)
    ldw r0 r8 r7

    ; non-zero address means the syscall is supported
    bool r0 r0
    ret



; ==========================================================
; void __syscall(...);
; ==========================================================
; The syscall handler.
;
; The system call number is passed in r9. Arguments to the syscall are passed
; in r0-r3 as normal.
; ==========================================================

@__syscall

    ; get the syscall into r7
    imw r8 ^__process_info_table
    ldw r8 rpp r8     ; r8 = process_info_table
    ldw r8 r8 8       ; r8 = syscall table
    shl r7 r9 3       ; r7 = byte offset of syscall (r9 << 3)
    add r7 r8 r7      ; r7 = address of syscall

    ; tail-call it
    ldw r9 r7 4       ; r9 = syscall context
    ldw rip r7 0      ; rip = syscall rip



; ==========================================================
; [[noreturn]] void __sys_exit(int exit_code);
; ==========================================================

=__sys_exit
    mov r9 0
    jmp ^__syscall



; ==========================================================
; int __sys_time(unsigned buffer[3]);
; ==========================================================

=__sys_time
    mov r9 1
    jmp ^__syscall



; ==========================================================
; int __sys_panic(int exit_code);
; ==========================================================

=__sys_panic
    mov r9 2
    jmp ^__syscall



; ==========================================================
; int __sys_fopen(const char* path, bool writeable);
; ==========================================================

=__sys_fopen
    mov r9 3
    jmp ^__syscall



; ==========================================================
; int __sys_fclose(int handle);
; ==========================================================

=__sys_fclose
    mov r9 4
    jmp ^__syscall



; ==========================================================
; int __sys_fread(int handle, void* out_buffer, unsigned size);
; ==========================================================

=__sys_fread
    mov r9 5
    jmp ^__syscall



; ==========================================================
; void __sys_fwrite(int handle, const void* buffer, unsigned size);
; ==========================================================

=__sys_fwrite
    mov r9 6
    jmp ^__syscall



; ==========================================================
; void __sys_fseek(int handle, unsigned position_low, unsigned position_high);
; ==========================================================

=__sys_fseek
    mov r9 7
    jmp ^__syscall



; ==========================================================
; void __sys_ftell(int handle, unsigned out_position[2]);
; ==========================================================

=__sys_ftell
    mov r9 8
    jmp ^__syscall



; ==========================================================
; void __sys_ftrunc(int handle, unsigned position_low, unsigned position_high);
; ==========================================================

=__sys_ftrunc
    mov r9 9
    jmp ^__syscall



; ==========================================================
; void __sys_stat(const char* path, unsigned out_stat[4]);
; ==========================================================

=__sys_stat
    mov r9 13
    jmp ^__syscall



; ==========================================================
; void __sys_rename(const char* from, const char* to);
; ==========================================================

=__sys_rename
    mov r9 14
    jmp ^__syscall



; ==========================================================
; void __sys_symlink(const char* from, const char* to);
; ==========================================================

=__sys_symlink
    mov r9 15
    jmp ^__syscall



; ==========================================================
; void __sys_unlink(const char* path);
; ==========================================================

=__sys_unlink
    mov r9 16
    jmp ^__syscall



; ==========================================================
; void __sys_chmod(const char* path, int mode);
; ==========================================================

=__sys_chmod
    mov r9 17
    jmp ^__syscall



; ==========================================================
; void __sys_mkdir(const char* path);
; ==========================================================

=__sys_mkdir
    mov r9 18
    jmp ^__syscall



; ==========================================================
; void __sys_rmdir(const char* path);
; ==========================================================

=__sys_rmdir
    mov r9 19
    jmp ^__syscall



; ==========================================================
; int __sys_dopen(const char* path);
; ==========================================================

=__sys_dopen
    mov r9 10
    jmp ^__syscall



; ==========================================================
; void __sys_dclose(int handle);
; ==========================================================

=__sys_dclose
    mov r9 11
    jmp ^__syscall



; ==========================================================
; int __sys_dread(int handle, char out_buffer[256]);
; ==========================================================

=__sys_dread
    mov r9 12
    jmp ^__syscall
