; The MIT License (MIT)
;
; Copyright (c) 2024 Fraser Heavy Software
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



; ==========================================================
; void __call_constructor(int argc, char** argv, char** envp, void* func);
; ==========================================================
; A helper to call a constructor function.
;
; opC doesn't have function pointers so we have to use this hack to call our
; constructor functions. The function to call is in r3; we just jump to it.
; ==========================================================

=__call_constructor
    mov rip r3



; ==========================================================
; void __call_destructor(void* func);
; ==========================================================
; A helper to call a constructor function.
;
; Same as above except we don't have any arguments. The function to call is in
; r0.
; ==========================================================

=__call_destructor
    mov rip r0
