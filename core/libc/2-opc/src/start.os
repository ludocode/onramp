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



; This implements the few startup/shutdown functions in assembly that can't be
; implemented in C. We defer as much as possible to C.



; ==========================================================
; [[noreturn]] void __start(uint32_t* process_info_vector);
; ==========================================================
; Initializes the libc and calls main().
;
; We forward control over to __start_c() which is implemented in C. This is
; just here to be included first by the linker to be the entry point of the
; program.
; ==========================================================

=__start
    "~Onr~amp~   "    ; The executable file identifier; see "File Format" in the VM spec
    mov r1 rsp
    jmp ^__start_c



; ==========================================================
; [[noreturn]] void __end(uint32_t exit_code, uint32_t exit_address);
; ==========================================================
; Jumps to the given exit address, thereby exiting the program with the given
; exit code.
; ==========================================================

=__end
    mov rip r1



; ==========================================================
; [[noreturn]] void abort(void);
; ==========================================================
; Halts the Onramp VM.
;
; This aborts not only the current process but all parent processes as well. If
; the program calls abort it's usually because they've encountered a bug and
; they are in an inconsistent state. There is no safe way to close our open
; files to let our parent process continue so we have no choice but to take
; down the whole VM.
;
; We do this simply by running an illegal instruction. If the VM is hosted it
; will exit with code 125.
; ==========================================================

=abort
    'FF 'FF 'FF 'FF



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
; A helper to call a constructor function.
;
; Same as above except we don't have any arguments. The program-relative
; function to call is in r0.
; ==========================================================

=__call_destructor
    add rip rpp r0
