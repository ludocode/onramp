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



; Generates all solutions to the 8 queens problem, printing them to standard
; output.



;==========================================
; int main(void);
;==========================================

=main

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ^^^
    add rfp rsp '00     ; mov rfp rsp

    ; call place_queen(0)
    add r0 '00 '00
    ims ra <place_queen
    ims ra >place_queen
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return 0
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ^^^
    add r0 '00 '00      ; zero r0
    ldw rip '00 rsp     ; ret



; ======================================
; char queens[8];
; ======================================
; An array of queen y positions. (The index is x.)
; ======================================

=queens
    'FF 'FF 'FF 'FF 'FF 'FF 'FF 'FF



; ======================================
; bool is_queen_valid(int x, int y);
; ======================================
; Returns true if a queen in the given row and column would be valid given
; the queens in the preceding columns, or false otherwise
;
; vars:
; - x: 0
; - y: 1
; - i: r2      ; iterating over columns < x
; - diff: r3   ; difference x-i (for checking diagonals)
; - q: r4      ; queen position in column i
; ======================================

=is_queen_valid
    ; don't bother with a stack frame

    ; iterate i from 0 to x
    add r2 '00 '00
:is_queen_valid_next_i

    ; if i==x, we've passed all tests, return true.
    sub ra r2 r0
    jz ra &is_queen_valid_done
    jz '00 &is_queen_valid_not_done
:is_queen_valid_done
    add r0 '00 '01
    ldw rip '00 rsp     ; ret
:is_queen_valid_not_done

    ; calculate the difference in x for checking diagonals
    sub r3 r0 r2

    ; get the queen's position in column i
    ims r8 <queens
    ims r8 >queens
    add r8 rpp r8
    ldb r4 r8 r2

    ; check if there's a queen in the same row
    sub ra r4 r1
    jz ra &is_queen_valid_fail

    ; check if there's a queen diagonally up
    add ra r1 r3
    sub ra r4 ra
    jz ra &is_queen_valid_fail

    ; check if there's a queen diagonally down
    sub ra r1 r3
    sub ra r4 ra
    jz ra &is_queen_valid_fail

    ; next i
    add r2 r2 '01    ; inc r2
    jz '00 &is_queen_valid_next_i

:is_queen_valid_fail
    ; return false
    add r0 '00 '00
    ldw rip '00 rsp     ; ret



; ======================================
; void place_queen(int x)
; ======================================
; Places a queen at all valid positions in the given column, recursing for each
; to place the next column.
;
; If the column number is 8, this instead prints the board.
;
; vars:
; - x: r0, rfp-4
; - y: r1, rfp-8
; ======================================

=place_queen

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ^^^
    add rfp rsp '00     ; mov rfp rsp
    sub rsp rsp '08

    ; if x is 8, we print the board instead of placing queens.
    sub ra r0 '08
    jz ra &place_queen_print
    jz '00 &place_queen_not_print
    
:place_queen_print

        ; clean up stack frame
        add rsp rfp '00     ; mov rsp rfp    ; leave
        ldw rfp '00 rsp     ; pop rfp
        add rsp rsp '04     ; ^^^

        ; tail-call print_board()
        ims ra <print_board    ; jmp ^print_board
        ims ra >print_board    ; ^^^
        add rip rpp ra         ; ^^^

:place_queen_not_print

    ; initialize vars
    stw r0 rfp 'FC     ; store x
    stw '00 rfp 'F8    ; store y=0

:place_queen_next_y

    ; check if a queen in this position is valid, call is_queen_valid(x,y)
    ldw r0 rfp 'FC    ; load x
    ldw r1 rfp 'F8    ; load y
    ims ra <is_queen_valid
    ims ra >is_queen_valid
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; if not valid, don't recurse
    jz r0 &place_queen_inc_y

    ; queen is valid. store it
    ldw r0 rfp 'FC    ; load x
    ldw r1 rfp 'F8    ; load y
    ims r8 <queens    ; imw r8 ^queens
    ims r8 >queens    ; ^^^
    add r8 rpp r8
    stb r1 r8 r0     ; queens[x] = y

    ; recurse place_queen(x+1)
    add r0 r0 '01      ; inc r0
    ims ra <place_queen
    ims ra >place_queen
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

:place_queen_inc_y
    ; inc y
    ldw r1 rfp 'F8    ; load y
    add r1 r1 '01     ; inc r1
    stw r1 rfp 'F8    ; store y

    ; next y
    sub ra r1 '08
    jz ra &place_queen_done
    jz '00 &place_queen_next_y

:place_queen_done
    ; done
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ^^^
    ldw rip '00 rsp     ; ret



; ======================================
; void print_board(void)
; ======================================
; vars:
; - x: r3
; - y: r4
; ======================================

=print_board

    ; set up a stack frame
    sub rsp rsp '04     ; push rfp       ; enter
    stw rfp '00 rsp     ; ^^^
    add rfp rsp '00     ; mov rfp rsp
    sub rsp rsp '08

    ; iterate over y from 0 to 8
    stw '00 rfp 'F8    ; y=0
:print_board_next_y

        ; iterate over x from 0 to 8
        stw '00 rfp 'FC    ; x=0
        :print_board_next_x

            ; check whether there is a queen in this position.
            ldw r3 rfp 'FC
            ims r8 <queens
            ims r8 >queens
            add r8 rpp r8
            ldb r7 r8 r3   ; r7 = queens[x]
            ldw r4 rfp 'F8
            sub r7 r7 r4   ; if r7 == y

            ; choose the character to emit, either "Q" or "."
            jz r7 &print_board_queen
            add r0 '00 "."    ; it's not a queen, print '.'
            jz '00 &print_board_char
        :print_board_queen
            add r0 '00 "Q"    ; it's a queen, print 'Q'
        :print_board_char

            ; call putchar()
            ims ra <putchar
            ims ra >putchar
            sub rsp rsp '04     ; push return address
            add rb rip '08
            stw rb '00 rsp
            add rip rpp ra    ; jump
            add rsp rsp '04     ; pop return address

            ; increment x
            ldw r3 rfp 'FC
            add r3 r3 '01   ; inc r3
            stw r3 rfp 'FC

            ; break if x == 8
            sub r7 r3 '08
            jz r7 &print_board_done_x

            ; print a space for alignment, call putchar(' ')
            add r0 '00 " "
            ims ra <putchar
            ims ra >putchar
            sub rsp rsp '04     ; push return address
            add rb rip '08
            stw rb '00 rsp
            add rip rpp ra    ; jump
            add rsp rsp '04     ; pop return address

            ; next
            jz '00 &print_board_next_x
        :print_board_done_x

        ; print newline, call putchar('\n')
        add r0 '00 '0A  ; \n
        ims ra <putchar
        ims ra >putchar
        sub rsp rsp '04     ; push return address
        add rb rip '08
        stw rb '00 rsp
        add rip rpp ra    ; jump
        add rsp rsp '04     ; pop return address

        ; next y
        ldw r4 rfp 'F8
        add r4 r4 '01   ; inc r4
        stw r4 rfp 'F8
        sub r7 r4 '08
        jz r7 &print_board_done_y
        jz '00 &print_board_next_y
:print_board_done_y

    ; call putchar('\n')
    add r0 '00 '0A  ; \n
    ims ra <putchar
    ims ra >putchar
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; call putchar('\n') again
    add r0 '00 '0A  ; \n
    ims ra <putchar
    ims ra >putchar
    sub rsp rsp '04     ; push return address
    add rb rip '08
    stw rb '00 rsp
    add rip rpp ra    ; jump
    add rsp rsp '04     ; pop return address

    ; return
    add rsp rfp '00     ; mov rsp rfp    ; leave
    ldw rfp '00 rsp     ; pop rfp
    add rsp rsp '04     ; ^^^
    ldw rip '00 rsp     ; ret
