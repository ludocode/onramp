; The MIT License (MIT)
;
; Copyright (c) 2023-2025 Fraser Heavy Software
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



; This implements the few startup/shutdown functions in assembly that can't be
; implemented in C. We defer as much as possible to C.



; ==========================================================
; [[noreturn]] void __start(uint32_t* process_info_table);
; ==========================================================
; Initializes the libc and calls main().
;
; We check the VM version and then forward control over to __start_c() which is
; implemented in C.
;
; The VM starts executing code at the first instruction, so the first function
; in a program is its entry point. This is the first function in this
; translation unit; this translation unit is the first in the libc archive; and
; the linker links the libc first, thus ensuring that this is the entry point
; of any program linked against the Onramp libc.
;
; This is the only place in Onramp where the VM version is checked. We don't
; bother to check the version during bootstrapping. The version checks are
; mainly to protect against a precompiled program being transferred from one PC
; to another with a different VM.
; ==========================================================

=__start
    ; The executable file identifier; see "File Format" in the VM spec.
    "~Onr~amp~   "

    ; We have to check the version before we do anything else. The VM spec has
    ; gone through several versions in which the instruction set and syscall
    ; mechanism have changed so it is not safe to run compiled C code until we
    ; know we have a supported VM. (We can't even safely do a less-than check
    ; because the comparison instruction has changed!)

    ; Version history:
    ; - 0: last experimental version before numbering
    ; - 1: replaced cmpu instruction with ltu
    ; - 2: replaced exit address with syscall table at pit[2]
    ; - 3: added pit count and syscall count to pit

    ; Check that VM version is 3
    ldw r1 r0 0
    sub r2 r1 3
    jnz r2 &__start_version_fail

    ; Version is good, jump into C
    mov r1 rsp
    jmp ^__start_c

:__start_version_fail
    ; Version check failed. We need to see if we recognize the version. We
    ; can't use ltu until we've confirmed the version is at least 1.

    ; Check VM version 0
    jnz r1 &__start_vm_not_0
    jmp ^__vm_version_0_or_1
:__start_vm_not_0

    ; Check VM version 1
    sub r2 r1 1
    jnz r2 &__start_vm_not_1
    jmp ^__vm_version_0_or_1
:__start_vm_not_1

    ; Version 2 or unknown version
    jmp ^__vm_version_other



; ==========================================================
; [[noreturn]] void __end(uint32_t exit_code, uint32_t exit_address);
; ==========================================================
; Jumps to the given exit address, thereby exiting the program with the given
; exit code.
;
; TODO remove this once exit syscall is working
; ==========================================================

=__end
    mov rip r1



; ==========================================================
; [[noreturn]] void abort(void);
; ==========================================================
; Halts the Onramp VM.
;
; This aborts not only the current process but all parent processes as well. If
; the program calls abort it's usually because it has encountered a bug and
; is in an inconsistent state. There is no safe way to close our open files to
; let our parent process continue so we have no choice but to take down the
; whole VM.
;
; We do this simply by running an illegal instruction. If the VM is hosted it
; will exit with code 125.
; ==========================================================

=abort
    ; TODO call panic

    ; Use an invalid instruction in a loop in case the program doesn't exit.
:__abort_loop
    'FF 'FF 'FF 'FF
    jmp &__abort_loop



; ==========================================================
; void __call_constructor(int argc, char** argv, char** envp, void* func);
; ==========================================================
; A helper to call a constructor function.
;
; opC doesn't have function pointers so we have to use this hack to call our
; constructor functions. The program-relative function to call is in r3; we
; just jump to it.
; ==========================================================

=__call_constructor
    add rip rpp r3



; ==========================================================
; void __call_destructor(void* func);
; ==========================================================
; A helper to call a destructor function.
;
; Same as above except we don't have any arguments. The program-relative
; function to call is in r0.
; ==========================================================

=__call_destructor
    add rip rpp r0



; ==========================================================
; char __vm_version_error_message[];
; ==========================================================
; An error message indicating that the VM version is not supported.
; ==========================================================

=__vm_version_error_message
    "ERROR: The Onramp VM version is not supported by this program." '0A '00



; ==========================================================
; [[noreturn]] void __vm_version_0_or_1(uint32_t* process_info_table);
; ==========================================================
; Exits on VM version 0 or 1 with an error indicating that the VM version is
; not supported.
;
; The only difference between VM versions 0 and 1 is the cmpu/ltu instruction.
; As long as we avoid it here we can use the same error handler on both
; versions.
;
; There aren't automated tests for old VM versions so don't change this without
; lots of manual testing.
;
; vars:
; - rfp-4: process_info_table
; - rfp-8: stderr handle
; - rfp-12: string pointer (initially __vm_version_error_message)
; - rfp-16: length remaining
; ==========================================================

=__vm_version_0_or_1

    ; set up a stack frame
    ; (c-debugger started quashing registers during syscalls in version 1 so we
    ; have to preserve them.)
    enter
    sub rsp rsp 16

    ; get the stderr handle
    stw r0 rfp -4
    ldw r0 r0 20    ; r0 = process_info_table[STDERR_HANDLE];
    stw r0 rfp -8

    ; get string pointer
    imw r1 ^__vm_version_error_message
    add r1 rpp r1
    stw r1 rfp -12

    ; count bytes in string (not safe to call strlen)
    zero r2
:__vm_version_0_count_loop
    ldb r3 r1 r2
    jz r3 &__vm_version_0_count_done
    inc r2
    jmp &__vm_version_0_count_loop
:__vm_version_0_count_done
    stw r2 rfp -16

    ; VM versions prior to 2 used a syscall instruction instead of a syscall
    ; function pointer table. The syscalls also had a different numbering.
    ; We have to loop to write all the bytes.
:__vm_version_0_write_loop
    '7F '06 '00 '00    ; sys fwrite 0 0

    ; if no bytes were written, or if an error occurs, we're done
    jz r0 &__vm_version_0_write_done
    shru r3 r0 31                      ; check high bit for write error
    jnz r3 &__vm_version_0_write_done

    ; increment string pointer
    ldw r1 rfp -12
    add r1 r1 r0
    stw r1 rfp -12

    ; decrement count
    ldw r2 rfp -16
    sub r2 r2 r0
    stw r2 rfp -16

    ; if no bytes left we're done
    jz r2 &__vm_version_0_write_done

    ; write again
    ldw r0 rfp -8
    jmp &__vm_version_0_write_loop

:__vm_version_0_write_done
    ; In VM versions prior to 2, the program exited by jumping to the exit
    ; address in the process info table.
    mov r0 1       ; exit code 1
    ldw r9 rfp -4  ; r9 = process_info_table
    ldw rip r9 8   ; rip = process_info_table[EXIT_ADDRESS]

    ; Use an invalid instruction in a loop in case the program doesn't exit.
:__vm_version_0_exit_loop
    'FF 'FF 'FF 'FF
    jmp &__vm_version_0_exit_loop



; ==========================================================
; [[noreturn]] void __vm_version_other(uint32_t* process_info_table);
; ==========================================================
; Exits with an error indicating that the VM version is not supported.
;
; This is called on VM version 2, or VM versions higher than expected. It
; prints an error message and exits through the system call table.
;
; This won't work correctly if anything used below is changed. Hopefully future
; VM versions will remain backwards compatible with the below instructions.
;
; There aren't automated tests for unsupported VM versions so don't change this
; without lots of manual testing.
;
; vars:
; - rfp-4: process_info_table
; - rfp-8: stderr handle
; - rfp-12: string pointer (initially __vm_version_error_message)
; - rfp-16: length remaining
; - rfp-20: rpp
; ==========================================================

=__vm_version_other

    ; set up a stack frame
    enter
    sub rsp rsp 20
    stw r0 rfp -4
    stw rpp rfp -20

    ; get the stderr handle
    ldw r0 r0 16    ; r0 = process_info_table[STDERR_HANDLE];
    stw r0 rfp -8

    ; get the error message
    imw r0 ^__vm_version_error_message
    add r0 rpp r0
    stw r0 rfp -12

    ; call strlen(__vm_version_error_message)
    call ^strlen
    stw r0 rfp -16

:__vm_version_other_write_loop

    ; get the fwrite syscall
    ldw r0 rfp -4  ; r0 = process_info_table
    ldw r5 r0 8    ; r5 = process_info_table[SYSCALL_TABLE]
    ldw r4 r5 48   ; r4 = syscall_table[SYS_FWRITE] (rip)

    ; call it (via foreign function call)
    ldw r0 rfp -8   ; r0 = stderr
    ldw r1 rfp -12  ; r1 = string pointer
    ldw r2 rfp -16  ; r2 = remaining bytes
    ldw r9 r5 52    ; r9 = syscall_table[SYS_FWRITE] (r9)
    call r4

    ; if no bytes were written, or if an error occurs, we're done
    jz r0 &__vm_version_other_write_done
    shru r3 r0 31                      ; check high bit for write error
    jnz r3 &__vm_version_other_write_done

    ; increment string pointer
    ldw r1 rfp -12
    add r1 r1 r0
    stw r1 rfp -12

    ; decrement count
    ldw r2 rfp -16
    sub r2 r2 r0
    stw r2 rfp -16

    ; if no bytes left we're done
    jz r2 &__vm_version_other_write_done

    ; write again
    jmp &__vm_version_other_write_loop

:__vm_version_other_write_done

    ; get the exit syscall
    ldw r0 rfp -4  ; r0 = process_info_table
    ldw r5 r0 8    ; r5 = process_info_table[SYSCALL_TABLE]
    ldw r4 r5 24   ; r4 = syscall exit (rip)
    ldw r9 r5 4    ; r9 = syscall exit (rip)

    ; call exit(1)
    ; (Note: if the VM version is newer than supported we can't guarantee that
    ; we haven't corrupted the parent's memory. We assume that future VM
    ; versions will remain reasonably compatible enough to make this safe.)
    mov r0 1
    call r4

    ; Use an invalid instruction in a loop in case the program doesn't exit.
:__vm_version_other_exit_loop
    'FF 'FF 'FF 'FF
    jmp &__vm_version_other_exit_loop
