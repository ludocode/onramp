#!/bin/sh

# Copyright (c) 2023-2026 Fraser Heavy Software
# Copyright (c) 2026 Laurent Huberdeau
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This is an experimental pure POSIX shell implementation of the Onramp virtual
# machine. It's mostly just a proof of concept at this point, too slow to be
# useful. It would take a week or more to complete the bootstrap on a modern
# PC.
#
# I wrote this in part to prove that you really can implement the Onramp VM in
# "anything" (not particularly surprising), but also in part because the shell
# is one of the few programming languages available out-of-the-box on a POSIX
# system. So this could have in theory allowed you to bootstrap a modern C
# compiler on any old POSIX system that has nothing else besides a shell and
# coreutils.
#
# Of course if it's too slow then it doesn't actually work, so this is probably
# all pointless. Given the difficulty of getting this working, it was actually
# much easier to write a VM directly in x86_64 machine code.
#
# Nevertheless, the shell VM implements enough of the spec to (in theory)
# complete the bootstrap, and it is portable enough to work in many shells.
# This VM passes all tests in the following shells:
#
# - bash
# - busybox ash
# - busybox hush
# - dash
# - ksh (2000)
# - ksh93u+m
# - mksh
# - oksh
# - osh
# - yash
# - zsh
#
# The following shells currently fail due to missing or incomplete POSIX
# features (see test.sh for details):
#
# - nsh
# - posh
# - toybox sh



# Implementation details:
#
# This VM is based on some ideas from Laurent Huberdeau's pnut compiler which
# compiles C to POSIX shell:
#
#     https://github.com/udem-dlteam/pnut
#
# POSIX shell doesn't have arrays so we use dynamically named variables. _0,
# _4, _8, etc. are used for memory and r0 to r15 are used for registers.
#
# The VM relies heavily on arithmetic expressions, i.e. `$(( ... ))`, which are
# specified by POSIX. Your shell must support these for this VM to work. Some
# syntax does not work in all shells; for example `$(( ++I ))` works in ksh but
# not dash. We use `$(( I += 1 ))` instead.
#
# Memory addresses and registers are stored using variable assignment in
# arithmetic expressions (e.g. `: $(( _$i = $x ))`.) When the address or
# register is computed, the arithmetic expressions are nested. For example:
#     : $(( r$(( expr2 )) = expr1 ))
# This assigns the value of expr1 to the register named by expr2.
#
# Since a full variable is used for each word of memory, the memory overhead is
# probably quite significant, but it is necessary for even tiny programs to run
# in a tolerable amount of time. The full bootstrap currently completes in
# under 2 MB of memory usage, so if the VM inflates this by for example 32x
# (taking 128 bytes per memory variable) that's 64 MB of real memory usage for
# the bootstrap.
#
# POSIX specifies that arithmetic expressions only need to support the range of
# signed long, which on 32-bit platforms is 32 bits. To work around this, we
# `and` all values with 0xFFFFFFFF after doing any kind of arithmetic. On
# 32-bit shells, high values will be negative signed, and on 64-bit shells, all
# values will be non-negative. Almost all operations give exactly the same
# results, and those that don't are handled specially (see the implementation
# of "ltu" for example) so it works on both. As far as I can tell this is both
# the fastest and simplest way to make the shell independent of word size.
#
# We use 0x7F as an opcode for syscalls, same as most VMs. The address of a
# word containing 0x7F is used as the rip for syscalls and the syscall number
# is in r9.



# Performance notes:
#
# The main instruction loop uses only `if`, `[` and arithmetic expressions.
# Assuming these are builtins, the shell should run most instructions without
# forking.
#
# Unfortunately, the names and contents of all variables are decimal strings,
# which means every instruction does several decimal conversions. This is
# probably the biggest performance killer in all shells.
#
# Reads and writes to files may fork external processes (`od` for reads and
# `dd` for writes.) This is expensive but the bootstrap and libc buffer all I/O
# now.
#
# These are the shell runtimes from the last time I ran all tests. Most of the
# time is taken up by the eight queens test so I don't know how representative
# this is of the bootstrap.
#
#     dash          0.5.12       17s
#     ksh93u+m      git 2026-04  22s
#     busybox ash   1.36.1       24s
#     ksh (2000)    final        43s
#     bash          5.3.9        52s
#     oksh          git 2026-03  52s
#     mksh          59.c         57s
#     osh           0.37.0       57s
#     yash          2.61         57s
#     zsh           5.9          58s
#     busybox hush  1.36.1       2m19s
#
# Even the fastest shell is still hundreds of times too slow to be useful. The
# Python VM completes all tests in 1.7s (and most of that is its slow startup
# time) and it is still way too slow to do the bootstrap reasonably.
#
# Based on a partial test of the bootstrap, the shell VM under bash is roughly
# 5000x slower than the debugger. So if the debugger takes, say, two minutes to
# run the bootstrap, the shell VM would take over 7 days (and several times
# that to use it to bootstrap a native compiler.) This is likely to be an
# underestimate because pnut's analysis shows that some shells slow down the
# more variables are defined, and the later stages of bootstrap will use nearly
# 2 MB of RAM (~500k variables.)



#set -e
#set -vx



################################################################
# Compatibility
################################################################

# zsh is by far the biggest offender of not following POSIX rules.
#
# We need at the very least the options sh_word_split (POSIX word splitting on
# parameters) and c_precedences (the precedence of shifts is different.) We
# also need to disable alternate variable bases. For example:
#
#     : $(( A = 5 & 0x1 )) ; echo $A
#
# Without a compatibility mode, zsh prints "16#1". If you do `setopt c_bases`,
# it prints "0x1". I haven't found an option to disable this; you have to
# manually `typeset -i 10 A` to stop this which is not practical for
# dynamically generated variables.
#
# Instead we just do `emulate sh` which disables as much zsh weirdness as
# possible.

if [ -n "$ZSH_VERSION" ]; then
    emulate sh
    #setopt sh_word_split  # POSIX word splitting on parameters
    #setopt c_precedences  # POSIX arithmetic expression precedence
fi

# Bash works just fine in its normal mode but we set POSIX compatibility anyway
# to avoid accidentally depending on Bash extensions when using it for
# development. It doesn't seem to run any faster in POSIX mode.

if [ -n "$BASH_VERSION" ]; then
    set -o posix
fi



################################################################
# Common Functions
################################################################

fatal() {
    echo ERROR: "$@" >&2
    exit 125
}

ERROR_GENERIC=$((0xFFFFFFFF))
ERROR_NO_SUCH_PATH=$((0xFFFFFFFE))
ERROR_IO=$((0xFFFFFFFD))
ERROR_UNSUPPORTED=$((0xFFFFFFFC))
ERROR_TRY_LATER=$((0xFFFFFFFB))
ERROR_END_OF_FILE=$((0xFFFFFFFA))
ERROR_OVERFLOW=$((0xFFFFFFF9))
ERROR_IN_USE=$((0xFFFFFFF8))



################################################################
# Memory Backing
################################################################

# The start of mapped VM memory. This is arbitrary.
# Words are stored in shell variables, named like so _$((address)).
# Uninitialized memory reads as 0 as per POSIX.
MEMORY_START=$((0x10000))
MEMORY_END=$(($MEMORY_START + 1024 * 1024 * 16)) # 16 MiB

# Verifies that a memory address is valid. If it is not, the VM aborts.
check_address() {
    if [ $1 -lt $MEMORY_START ] || [ $1 -ge $MEMORY_END ]; then
        fatal "Invalid memory address: $1"
    fi
}

check_alignment() {
    if [ $(( $1 & 0x3 )) -ne 0 ]; then
        fatal "Invalid memory alignment: $1"
    fi
}

# Loads a byte at the given address and places it in LOAD_BYTE_RET.
load_byte() {
    #check_address $1
    LOAD_BYTE_IX=$(( $1 & 0xFFFFFFFC ))
    LOAD_BYTE_RET=$(( (_${LOAD_BYTE_IX} >> (($1 & 0x3) << 3)) & 0xFF ))
}

# Loads a word from the given address and places it in LOAD_WORD_RET.
load_word() {
    #check_address $1
    #check_alignment $1
    LOAD_WORD_RET=$(( _$1 ))
}

# Stores a byte at the given address.
store_byte() {
    # Check the memory address
    check_address $1
    STORE_BYTE_IX=$(( $1 & 0xFFFFFFFC ))
    STORE_BYTE_OFFSET=$(( ($1 & 0x3) << 3 ))
    : $(( _${STORE_BYTE_IX} &= ~(0xFF << STORE_BYTE_OFFSET) )) # clear the byte
    : $(( _${STORE_BYTE_IX} |= (($2 & 0xFF) << STORE_BYTE_OFFSET) )) # replace the byte
}

# Stores a word at the given address.
store_word() {
    #check_address $1
    #check_alignment $1
    : $(( _$1 = $2 & 0xFFFFFFFF ))
}

# Loads a null-terminated string into LOAD_STRING_RET
load_string() {
    LOAD_STRING_RET=
    I=$1
    while true; do
        load_byte $I
        if [ $LOAD_BYTE_RET -eq 0 ]; then
            break
        fi
        LOAD_STRING_RET="$LOAD_STRING_RET$(printf \\$(printf %o $LOAD_BYTE_RET))"
        : $(( I += 1 ))
    done
    #echo "loaded string: $LOAD_STRING_RET"
}



################################################################
# Registers
################################################################

# Registers are stored as 32-bit signed decimal strings.

registers_init() {
    #echo registers_init >&2
    I=0
    while [ $I -ne 16 ]; do
        : $(( r$I = 0 ))
        I=$(( $I + 1 ))
    done
}

register_get() {
    REGISTER_GET_RET=$(( r$1 ))
}

register_set() {
    : $(( r$1 = $2 ))
}

registers_print() {
    I=0
    while [ $I -ne 16 ]; do
        register_get $I
        echo "r$I = $((r$I)) (0x$(printf %08X $((r$I)))" >&2
        I=$(( $I + 1 ))
    done
}



################################################################
# I/O
################################################################

FILES_MAX=16

files_init() {
    #echo files_init >&2

    I=0
    while [ $I -ne $FILES_MAX ]; do
        #echo "$I" >&2
        eval FILE_${I}_NAME=
        : $(( FILE_${I}_SIZE = 0 ))
        : $(( FILE_${I}_OFFSET = 0 ))
        : $(( FILE_${I}_MODE = 0 ))
        : $(( FILE_${I}_STREAM = 0 ))
        : $(( I += 1 ))
    done

    FILE_0_NAME=/dev/stdin
    FILE_0_MODE=0
    FILE_0_STREAM=1

    FILE_1_NAME=/dev/stdout
    FILE_1_MODE=1
    FILE_1_STREAM=1

    FILE_2_NAME=/dev/stderr
    FILE_2_MODE=1
    FILE_2_STREAM=1
}

file_print() {
    echo "file $1:"
    eval echo "  name" \$FILE_${1}_NAME
    eval echo "  size" \$FILE_${1}_SIZE
    eval echo "  offset" \$FILE_${1}_OFFSET
    eval echo "  mode" \$FILE_${1}_MODE
    eval echo "  stream" \$FILE_${1}_STREAM
}

file_open() {

    # Find a free file handle
    I=0
    while [ "$(eval echo \$FILE_${I}_NAME)" != "" ]; do
        if [ $I -eq $FILES_MAX ]; then
            #echo "Couldn't open \"$1\" in mode $2; too many open files." >&2
            # too may open files
            FILE_OPEN_RET=$ERROR_OVERFLOW
            return
        fi
        : $(( I += 1 ))
    done

    #echo "Opening \"$1\" in mode $2 as handle $I" >&2

    # Check if this is a device
    case "$1" in
        /dev/*)
            # We only support /dev/urandom. No other devices are allowed.
            if [ "$1" != "/dev/urandom" ]; then
                echo "VM WARNING: Refusing to open device \"$1\"" >&2
                FILE_OPEN_RET=$ERROR_NO_SUCH_PATH
                return
            fi
            # Devices are not seekable.
            : $(( FILE_${I}_STREAM = 1 ))
            ;;
        *)
            # Assume the file is seekable. This won't work on UNIX domain
            # sockets.
            : $(( FILE_${I}_STREAM = 0 ))
            ;;
    esac

    # Open for reading
    if [ $2 -eq 0 ]; then
        if ! [ -e "$1" ]; then
            FILE_OPEN_RET=$ERROR_NO_SUCH_PATH
            return
        fi
        : $(( FILE_${I}_SIZE = $(wc -c < "$1") ))
        : $(( FILE_${I}_OFFSET = 0 ))

    # Open for writing
    elif [ $2 -eq 1 ]; then
        if [ -e "$1" ]; then
            : $(( FILE_${I}_SIZE = $(wc -c < "$1") ))
        else
            touch "$1"
            : $(( FILE_${I}_SIZE = 0 ))
        fi
        : $(( FILE_${I}_OFFSET = 0 ))

    else
        fatal "Invalid file mode"
    fi

    eval FILE_${I}_NAME="\$1"
    : $(( FILE_${I}_MODE = $2 ))
    FILE_OPEN_RET=$I
}

file_close() {
    #echo "Closing handle $1" >&2

    if [ "x$(eval echo \$FILE_${1}_NAME)" = "x" ]; then
        fatal "Handle is already closed."
    fi

    eval FILE_${1}_NAME=
}

# Reads bytes from the given file into the given address.
# args: handle, address, count
file_read() {
    FILE_READ_NAME="$(eval echo "\$FILE_${1}_NAME")"
    FILE_READ_ADDRESS=$2
    FILE_READ_COUNT=$3
    : $(( FILE_READ_OFFSET = FILE_${1}_OFFSET ))
    : $(( FILE_READ_SIZE = FILE_${1}_SIZE ))
    : $(( FILE_READ_STREAM = FILE_${1}_STREAM ))

    #echo "file read handle $1" >&2
    #echo "file read name $FILE_READ_NAME" >&2
    #echo "file read address $FILE_READ_ADDRESS" >&2
    #echo "file read count $FILE_READ_COUNT" >&2
    #echo "file read offset $FILE_READ_OFFSET" >&2
    #echo "file read size $FILE_READ_SIZE" >&2

    if [ "$FILE_READ_NAME" = "" ]; then
        fatal "Handle $1 is not open."
    fi

    # Make sure we don't try to read past the end of the file
    FILE_READ_REMAINING=$(( FILE_READ_SIZE - FILE_READ_OFFSET ))
    if [ $FILE_READ_REMAINING -eq 0 ]; then
        #echo "file read EOF" >&2
        FILE_READ_RET=$ERROR_END_OF_FILE
        return
    fi
    if [ $FILE_READ_REMAINING -lt $FILE_READ_COUNT ]; then
        FILE_READ_COUNT=$FILE_READ_REMAINING
        #echo "file read reduced count to $FILE_READ_COUNT" >&2
    fi

    # We're going to read into a hex string so we limit ourselves to 512 bytes.
    if [ $FILE_READ_COUNT -gt 512 ]; then
        FILE_READ_COUNT=512
    fi

    # If the file is seekable, od will seek to the offset
    if [ $FILE_READ_STREAM -eq 0 ]; then
        FILE_READ_SEEK=-j$FILE_READ_OFFSET
    else
        FILE_READ_SEEK=
    fi

    # Read hex bytes into a string. (They are grouped multiple bytes per line
    # but this doesn't matter because both space and newline are in our IFS.)
    FILE_READ_BYTES=$(od -An -vtx1 $FILE_READ_SEEK -N$FILE_READ_COUNT "$FILE_READ_NAME")

    # old code to delimit bytes by newlines
#    FILE_READ_BYTES=$( \
#        od -An -vtx1 $FILE_READ_SEEK -N$FILE_READ_COUNT "$FILE_READ_NAME" | \
#        LC_ALL=C tr -cs '0-9a-fA-F' '[\n*]' | \
#        grep .)

    # Store each byte in memory.
    : $(( FILE_READ_END = FILE_READ_ADDRESS + FILE_READ_COUNT ))
    for FILE_READ_BYTE in $FILE_READ_BYTES; do  # zsh needs sh_word_split here
        #echo "read byte $FILE_READ_BYTE"
        store_byte $FILE_READ_ADDRESS 0x$FILE_READ_BYTE
        : $(( FILE_READ_ADDRESS += 1 ))
        : $(( FILE_READ_OFFSET += 1 ))
    done
    if [ $FILE_READ_END -ne $FILE_READ_ADDRESS ]; then
        fatal "Read incorrect number of bytes."
    fi

    # (We don't have any error checking on the above. If the size is wrong, for
    # example if the file was modified by something else since we "opened" it,
    # this will blow up.)

    : $(( FILE_${1}_OFFSET = FILE_READ_OFFSET ))
    FILE_READ_RET=$FILE_READ_COUNT
}

# An internal helper for file_write.
file_write_loop() {
    I=0
    while [ $I -ne $FILE_WRITE_COUNT ]; do
        load_byte $(( $FILE_WRITE_ADDRESS + $I ))
        printf \\$(printf %o $LOAD_BYTE_RET)
        : $(( I += 1 ))
    done
}

# Writes bytes at the given address to the given file.
# args: handle, address, count
file_write() {

    # TODO this is probably not correct handling of filenames with spaces.
    FILE_WRITE_NAME="$(eval echo "\$FILE_${1}_NAME")"
    FILE_WRITE_ADDRESS=$2
    FILE_WRITE_COUNT=$3
    : $(( FILE_WRITE_MODE = FILE_${1}_MODE ))
    : $(( FILE_WRITE_OFFSET = FILE_${1}_OFFSET ))
    : $(( FILE_WRITE_STREAM = FILE_${1}_STREAM ))

    #echo "file write handle $1" >&2
    #echo "file write name $FILE_WRITE_NAME" >&2
    #echo "file write address $FILE_WRITE_ADDRESS" >&2
    #echo "file write count $FILE_WRITE_COUNT" >&2
    #echo "file write offset $FILE_WRITE_OFFSET" >&2
    #echo "file write size $FILE_WRITE_SIZE" >&2

    if [ "$FILE_WRITE_NAME" = "" ]; then
        fatal "Handle is not open."
    fi
    if [ $FILE_WRITE_MODE -ne 1 ]; then
        fatal "Handle is not open for writing."
    fi

    FILE_WRITE_SEEK=
    if [ $FILE_WRITE_STREAM -eq 0 ]; then
        FILE_WRITE_SEEK="seek=$FILE_WRITE_OFFSET"
    fi

    # We duplicate the loop rather than piping it the if statement because
    # piping creates a subprocess, and we need to avoid forking as much as
    # possible for performance. When writing to files we have to call out to dd
    # which is not usually a builtin (busybox and toybox are exceptions), but
    # file writes are entirely buffered in the libc and bootstrap process.
    if [ "$FILE_WRITE_NAME" = "/dev/stdout" ]; then
        file_write_loop
    elif [ "$FILE_WRITE_NAME" = "/dev/stderr" ]; then
        file_write_loop >&2
    else
        file_write_loop | dd \
                bs=1 \
                count=$FILE_WRITE_COUNT \
                of="$FILE_WRITE_NAME" \
                $FILE_WRITE_SEEK \
                conv=notrunc 2>/dev/null
    fi

    # Extend the offset, and the size if we've gone past the end
    if [ $FILE_WRITE_STREAM -eq 0 ]; then
        : $(( FILE_${1}_OFFSET += FILE_WRITE_COUNT ))
        if [ $(( FILE_${1}_OFFSET )) -gt $(( FILE_${1}_SIZE )) ]; then
            : $(( FILE_${1}_SIZE = FILE_${1}_OFFSET ))
        fi
    fi

    FILE_WRITE_RET=$FILE_WRITE_COUNT
}

# args: handle, base, offset
# (to differentiate between the two offsets, we call them "target" and "current".)
file_seek() {
    FILE_SEEK_FILENAME="$(eval echo \$FILE_${1}_NAME)"
    FILE_SEEK_BASE=$2
    FILE_SEEK_TARGET=$3
    : $(( FILE_SEEK_CURRENT = FILE_${1}_OFFSET ))
    : $(( FILE_SEEK_SIZE = FILE_${1}_SIZE ))
    : $(( FILE_SEEK_STREAM = FILE_${1}_STREAM ))

    if [ "$FILE_SEEK_FILENAME" = "" ]; then
        fatal "Handle is not open."
    fi

    if [ $FILE_SEEK_STREAM -eq 1 ]; then
        # cannot seek stream
        FILE_SEEK_RET=$ERROR_UNSUPPORTED
        return
    fi

    # TODO these can probably be removed, only syscall seek uses this and it
    # doesn't take a base parameter anymore
    if [ $FILE_SEEK_BASE -eq 1 ]; then
        : $(( FILE_SEEK_TARGET += FILE_SEEK_CURRENT ))
    elif [ $FILE_SEEK_BASE -eq 2 ]; then
        : $(( FILE_SEEK_TARGET = ((FILE_SEEK_SIZE + FILE_SEEK_TARGET) & 0xFFFFFFFF) ))
    fi

    if [ $FILE_SEEK_TARGET -gt $FILE_SEEK_SIZE ]; then
        # TODO overflow is not documented here, should update the spec
        FILE_SEEK_RET=$ERROR_OVERFLOW
        return
    fi

    : $(( FILE_${1}_OFFSET = FILE_SEEK_TARGET ))
    FILE_SEEK_RET=0
}

# args: handle
file_size() {
    FILE_SIZE_FILENAME="$(eval echo \$FILE_${1}_NAME)"
    : $(( FILE_SIZE_STREAM = FILE_${1}_STREAM ))

    if [ "$FILE_SIZE_FILENAME" = "" ]; then
        fatal "Handle is not open."
    fi

    if [ $FILE_SIZE_STREAM -eq 1 ]; then
        # cannot seek stream
        FILE_SIZE_RET=$ERROR_UNSUPPORTED
        return
    fi

    # `du -b` is not POSIX; we have to use `wc -c` which might read the whole
    # file. It might also print leading spaces or use tabs as delimiter so we
    # use sed to clean them up.
    #     https://unix.stackexchange.com/a/747527
    FILE_SIZE_RET=$(wc -c "$FILE_SIZE_FILENAME" \
        | sed 's/\t/ /g' \
        | sed 's/^ *//' \
        | cut -d\  -f1)
}

# args: handle
file_trunc() {

    # TODO this is probably not correct handling of filenames with spaces.
    FILE_WRITE_NAME="$(eval echo "\$FILE_${1}_NAME")"
    : $(( FILE_WRITE_MODE = FILE_${1}_MODE ))
    : $(( FILE_WRITE_OFFSET = FILE_${1}_OFFSET ))
    : $(( FILE_WRITE_STREAM = FILE_${1}_STREAM ))

    #echo "file write handle $1" >&2
    #echo "file write name $FILE_WRITE_NAME" >&2
    #echo "file write address $FILE_WRITE_ADDRESS" >&2
    #echo "file write count $FILE_WRITE_COUNT" >&2
    #echo "file write offset $FILE_WRITE_OFFSET" >&2
    #echo "file write size $FILE_WRITE_SIZE" >&2

    if [ "$FILE_WRITE_NAME" = "" ]; then
        fatal "Handle is not open."
    fi
    if [ $FILE_WRITE_MODE -ne 1 ]; then
        fatal "Handle is not open for writing."
    fi

    >"$FILE_WRITE_NAME"
    : $(( FILE_${1}_OFFSET = 0 ))
    : $(( FILE_${1}_SIZE = 0 ))
}



################################################################
# Initialization Process
################################################################

# Returns the number of strings in STRING_COUNT_RET.
string_count() {
    STRING_COUNT_RET=$#
}

# Copies a list of strings into the VM at CURRENT_ADDRESS, returning the
# address of the array in COPY_STRINGS_RET.
copy_strings() {
    string_count "$@"

    # Reserve space for the array
    COPY_STRINGS_RET=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + ($STRING_COUNT_RET + 1) * 4 ))

    # For each string...
    I=0
    while [ $I -ne $STRING_COUNT_RET ]; do
        STR="$1"

        # Copy the string to the heap
        J=0
        while [ ! -z "$STR" ]; do
            CHR=${STR%"${STR#?}"}       # Get the first character
            STR=${STR#?}                # Remove the first character
            BYTE=$(printf %d "'$CHR")   # Get the byte value of the character
            store_byte $(( $CURRENT_ADDRESS + $J )) "$BYTE"
            J=$(($J + 1))
        done
        store_byte $(( $CURRENT_ADDRESS + $J )) 0

        # Write the string's address to the array
        store_word $(( $COPY_STRINGS_RET + $I * 4 )) $CURRENT_ADDRESS
        CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + $J + 1 ))

        shift
        I=$(( $I + 1 ))
    done

    # Write the null terminator
    store_word $(( $COPY_STRINGS_RET + $STRING_COUNT_RET * 4 )) 0

    # Align the current address
    CURRENT_ADDRESS=$(( ($CURRENT_ADDRESS + 3) & (0xFFFFFFFC) ))
}

# Inserts a syscall into the syscall table
syscall_enable() {
    #echo "enabling syscall $1" >&2
    store_word $(( $SYSCALL_TABLE_ADDRESS + $1 * 8 )) $SYSCALL_INSTRUCTION_ADDRESS
    store_word $(( $SYSCALL_TABLE_ADDRESS + $1 * 8 + 4 )) $1
}

syscalls_init() {
    #echo syscalls_init >&2

    # Allocate the syscall table
    SYSCALL_COUNT=25
    SYSCALL_TABLE_ADDRESS=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 8 * $SYSCALL_COUNT ))
    #echo "syscall table is at $SYSCALL_TABLE_ADDRESS" >&2

    # Generate the syscall instruction
    SYSCALL_INSTRUCTION_ADDRESS=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 4 ))
    store_word $SYSCALL_INSTRUCTION_ADDRESS $((0x7F))
    #echo "syscall instruction is at $SYSCALL_INSTRUCTION_ADDRESS" >&2

    # Fill the syscall table
    # TODO we should implement most of the below. those not implemented yet are
    # commented.
    syscall_enable 0   # exit
    #syscall_enable 2   # time
    syscall_enable 3   # open
    syscall_enable 4   # close
    syscall_enable 5   # read
    syscall_enable 6   # write
    syscall_enable 7   # seek
    syscall_enable 8   # tell
    syscall_enable 9   # trunc
    #syscall_enable 12  # dirent
    #syscall_enable 13  # stat
    #syscall_enable 14  # rename
    #syscall_enable 16  # delete
    #syscall_enable 17  # chmod
    syscall_enable 18  # mkdir

    # Write the syscall table address into the process info table
    store_word $(( $PROCESS_INFO_TABLE + 8 )) $SYSCALL_TABLE_ADDRESS
}

# Loads the process info vector and its contents (command-line arguments,
# environment variables, syscall table, etc.)
process_init() {
    #echo process_init >&2

    PROCESS_INFO_TABLE_COUNT=12

    # Allocate the process info table
    CURRENT_ADDRESS=$MEMORY_START
    PROCESS_INFO_TABLE=$CURRENT_ADDRESS
    CURRENT_ADDRESS=$(( $CURRENT_ADDRESS + 4 * $PROCESS_INFO_TABLE_COUNT ))

    # Fill the process info table
    store_word $PROCESS_INFO_TABLE 4                              # major version
    # heap start is set in program_init()
    # syscall table is set in syscalls_init()
    store_word $(( $PROCESS_INFO_TABLE + 4 * 3 )) 0               # stdin
    store_word $(( $PROCESS_INFO_TABLE + 4 * 4 )) 1               # stdout
    store_word $(( $PROCESS_INFO_TABLE + 4 * 5 )) 2               # stderr
    # command-line args are set in args_init
    # env vars are set in env_init
    store_word $(( $PROCESS_INFO_TABLE + 4 * 8 )) 0               # working directory TODO
    store_word $(( $PROCESS_INFO_TABLE + 4 * 9 )) 7               # capabilities = echo | blocking | line-oriented
    store_word $(( $PROCESS_INFO_TABLE + 4 * 10 )) 0              # minor version
    store_word $(( $PROCESS_INFO_TABLE + 4 * 11 )) 0              # additional memory regions
}

args_init() {
    #echo args_init >&2
    copy_strings "$@"
    store_word $(( $PROCESS_INFO_TABLE + 24 )) $COPY_STRINGS_RET   # argv
}

env_init() {
    #echo env_init >&2
    #copy_strings $(env)
COPY_STRINGS_RET=0  # TODO env_init not working yet
    store_word $(( $PROCESS_INFO_TABLE + 4 * 7 )) $COPY_STRINGS_RET   # environ
}

# Loads the program into memory and sets the heap start.
program_init() {
    #echo program_init >&2

    PROGRAM_FILENAME=$1
    if [ "x$PROGRAM_FILENAME" = "x" ]; then
        fatal "A program name is required."
    fi
    if ! [ -e "$PROGRAM_FILENAME" ]; then
        fatal "The program file doesn't exist."
    fi

    # Set initial registers
    PROGRAM_BASE_ADDRESS=$CURRENT_ADDRESS
    r0=$PROCESS_INFO_TABLE     # r0
    r12=$MEMORY_END            # rsp
    r14=$PROGRAM_BASE_ADDRESS  # rpp
    r15=$PROGRAM_BASE_ADDRESS    # rip

    # Load the program
    #echo "Loading program $PROGRAM_FILENAME" >&2
    # TODO check to make sure the program doesn't overflow our heap
    file_open $PROGRAM_FILENAME 0
    HANDLE=$FILE_OPEN_RET
    REMAINING=$(( FILE_${HANDLE}_SIZE ))
    while [ $REMAINING -gt 0 ]; do
        file_read $HANDLE $CURRENT_ADDRESS $REMAINING
        #echo "read $FILE_READ_RET bytes"
        if [ $(( $FILE_READ_RET & 0x80000000 )) -ne 0 ]; then
            fatal "Failed to read program."
        fi
        : $(( REMAINING -= FILE_READ_RET ))
        : $(( CURRENT_ADDRESS += FILE_READ_RET ))
    done
    file_close $HANDLE

    # Store the remaining memory as the heap start in the process info table
    store_word $(( $PROCESS_INFO_TABLE + 4 )) $(( $CURRENT_ADDRESS ))  # heap start

    #echo "Done loading program" >&2

    # Check for a wrap header ("#!" or "REM")
    load_byte $r15
    B0=$LOAD_BYTE_RET
    load_byte $(( r15 + 1))
    B1=$LOAD_BYTE_RET
    load_byte $(( r15 + 2))
    B2=$LOAD_BYTE_RET
    if ( [ $B0 -eq $((0x23)) ] && [ $B1 -eq $((0x21)) ] ) || \
        ( [ $B0 -eq $((0x52)) ] && [ $B1 -eq $((0x45)) ] && [ $B2 -eq $((0x4D)) ] ); then
        # wrap header found
        : $(( r14 += 128 ))
        : $(( r15 += 128 ))
    fi
}



################################################################
# Syscalls
################################################################

syscall_open() {
    # Read the filename from memory
    load_string $r0

    # Call open
    MODE=$r1
    file_open "$LOAD_STRING_RET" $MODE

    # Put the result in r0
    register_set 0 $FILE_OPEN_RET
}

syscall_close() {
    file_close $r0
    register_set 0 0
}

syscall_read() {
    HANDLE=$r0
    ADDRESS=$r1
    COUNT=$r2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid read size."
    fi

    file_read $HANDLE $ADDRESS $COUNT
    register_set 0 $FILE_READ_RET
}

syscall_write() {
    HANDLE=$r0
    ADDRESS=$r1
    COUNT=$r2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid write count."
    fi

    file_write $HANDLE $ADDRESS $COUNT
    register_set 0 $FILE_WRITE_RET
}

syscall_seek() {
    HANDLE=$r0
    OFFSET_LOW=$r1
    OFFSET_HIGH=$r2

    # We only support up to 2 GiB file size.
    if [ $OFFSET_HIGH -ne 0 ] || [ $OFFSET_LOW -lt 0 ] || [ $OFFSET_LOW -gt $((0x7FFFFFFF)) ]; then
        register_set 0 $ERROR_GENERIC
        return
    fi

    file_seek $HANDLE 0 $OFFSET_LOW
    register_set 0 $FILE_SEEK_RET
}

syscall_size() {
    HANDLE=$r0
    POSITION=$r1

    file_size $HANDLE

    # We only support up to 2 GiB file size.
    if [ $FILE_SIZE_RET -lt 0 ] || [ $FILE_SIZE_RET -gt $((0x7FFFFFFF)) ]; then
        register_set 0 $FILE_SIZE_RET
        return
    fi

    store_word $POSITION $FILE_SIZE_RET
    store_word $(( POSITION + 4 )) 0
    register_set 0 0
}

syscall_trunc() {
    HANDLE=$r0
    SIZE_LOW=$r1
    SIZE_HIGH=$r2

    # We only support truncating to zero.
    if [ $SIZE_LOW -ne 0 ] || [ $SIZE_HIGH -ne 0 ]; then
        register_set 0 $ERROR_UNSUPPORTED
        return
    fi

    # Attempting to truncate a non-writable handle is undefined behaviour. We
    # don't need to check for errors.
    file_trunc $HANDLE
    register_set 0 0
}

syscall_mkdir() {
    load_string $r0
    mkdir "$LOAD_STRING_RET" 2>/dev/null 1>&2
    : $(( r0 = ($? == 0) ? 0 : 0xFFFFFFFF ))
}

syscall() {
    case $r9 in
        0)  # halt
            exit $(( $r0 & 0xFF ))
            ;;
        3) syscall_open ;;
        4) syscall_close ;;
        5) syscall_read ;;
        6) syscall_write ;;
        7) syscall_seek ;;
        8) syscall_size ;;
        9) syscall_trunc ;;
        18) syscall_mkdir ;;
        *)
            fatal "Unhandled syscall: $1"
            ;;
    esac

    # load the top of the stack into the instruction pointer
    load_word $r12
    register_set 15 $LOAD_WORD_RET
}



################################################################
# Instruction Execution
################################################################

parse_register() {
    PARSE_REGISTER_RET=$(( $1 - 0x80 ))
    if [ $PARSE_REGISTER_RET -lt 0 -o $PARSE_REGISTER_RET -ge 16 ]; then
        fatal "A register-type argument is invalid."
    fi
}

parse_mix() {
    PARSE_MIX_RET=$(( $1 ))
    if [ $PARSE_MIX_RET -ge 144 ]; then
        PARSE_MIX_RET=$(( ($PARSE_MIX_RET - 256) & 0xFFFFFFFF ))
    elif [ $PARSE_MIX_RET -ge 128 ]; then
        register_get $(( $PARSE_MIX_RET - 0x80 ))
        PARSE_MIX_RET=$REGISTER_GET_RET
    fi
}

# Parses mix-type ARG1.
parse_mix_arg1() {
    if [ $ARG1 -ge 128 ]; then
        if [ $ARG1 -ge 144 ]; then
            : $(( ARG1 |= 0xFFFFFF00 ))
        else
            : $(( ARG1 = r$(( ARG1&0xF )) ))
        fi
    fi
}

# Parses mix-type ARG2.
parse_mix_arg2() {
    if [ $ARG2 -ge 128 ]; then
        if [ $ARG2 -ge 144 ]; then
            : $(( ARG2 |= 0xFFFFFF00 ))
        else
            : $(( ARG2 = r$(( ARG2&0xF )) ))
        fi
    fi
}

# Parses mix-type ARG3.
parse_mix_arg3() {
    if [ $ARG3 -ge 128 ]; then
        if [ $ARG3 -ge 144 ]; then
            : $(( ARG3 |= 0xFFFFFF00 ))
        else
            : $(( ARG3 = r$(( ARG3&0xF )) ))
        fi
    fi
}

run() {
    while true; do
        #echo "about to load instruction word at $r15"
        : $(( INSTRUCTION = _$r15 ))
        : $(( OPCODE      = INSTRUCTION & 0x0F ))
        : $(( ARG1        = (INSTRUCTION >> 8)  & 0xFF ))
        : $(( ARG2        = (INSTRUCTION >> 16) & 0xFF ))
        : $(( ARG3        = (INSTRUCTION >> 24) & 0xFF ))

        #echo >&2
        #echo "instruction at 0x$(printf %08X $((r15-PROGRAM_BASE_ADDRESS)) ):" \
        #    "$(printf %02X $((INSTRUCTION&0xFF))) $(printf %02X $ARG1) $(printf %02X $ARG2) $(printf %02X $ARG3)" >&2

        # check that opcode starts with 0x7x (disabled for performance)
#        if [ $((0x70)) -ne $(( $INSTRUCTION & 0xF0 )) ]; then
#            fatal "Invalid opcode"
#        fi

#        if [ $OPCODE = "00" ]; then
#            registers_print
#        fi

        : $(( r15 += 4 ))

        case $OPCODE in
            0)
                #echo add >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = (ARG2 + ARG3) & 0xFFFFFFFF ))
                ;;
            1)
                #echo sub >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = (ARG2 - ARG3) & 0xFFFFFFFF ))
                ;;
            2)
                #echo mul >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = (ARG2 * ARG3) & 0xFFFFFFFF ))
                ;;
            3)
                #echo divu >&2
                parse_mix_arg2
                parse_mix_arg3
                if [ $ARG2 -lt 0 -o $ARG3 -lt 0 ]; then
                    echo "VM ERROR: division out of bounds; TODO unsigned division is not properly implemented."
                    # Probably whichever are negative should be shifted down
                    # (unsigned) by one bit, then shift the result
                    # appropriately and do a test multiplication to correct it.
                    # I haven't bothered to set up a 32-bit shell to test with
                    # at the moment.
                fi
                : $(( r$(( ARG1&0xF )) = (ARG2 / ARG3) & 0xFFFFFFFF ))
                ;;
            4)
                #echo and >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = ARG2 & ARG3 ))
                ;;
            5)
                #echo or >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = ARG2 | ARG3 ))
                ;;
            6)
                #echo shl >&2
                parse_mix_arg2
                parse_mix_arg3
                : $(( r$(( ARG1&0xF )) = (ARG2 << ARG3) & 0xFFFFFFFF ))
                ;;
            7)
                #echo shru >&2
                parse_mix_arg2
                parse_mix_arg3
                # We only have a signed shift. We shift down by 1, mask out the
                # high bit, then shift the rest.
                : $(( r$(( ARG1&0xF )) = (ARG3 == 0) ? ARG2 :
                    ((($ARG2 >> 1) & 0x7FFFFFFF) >> ($ARG3 - 1)) ))
                ;;
            8)
                #echo ldw >&2
                parse_mix_arg2
                parse_mix_arg3
                #echo ldw at $ARG2 + $PARSE_MIX_RET into reg $PARSE_REGISTER_RET >&2
                : $(( r$(( ARG1&0xF )) = _$(( (ARG2 + ARG3) & 0xFFFFFFFF )) ))
                ;;
            9)
                #echo stw >&2
                parse_mix_arg1
                parse_mix_arg2
                parse_mix_arg3
                : $(( _$(( (ARG2 + ARG3) & 0xFFFFFFFF )) = $ARG1 ))
                ;;
            10)
                #echo ldb >&2
                parse_mix_arg2
                parse_mix_arg3
                load_byte $(( (ARG2 + ARG3) & 0xFFFFFFFF ))
                : $(( r$(( ARG1&0xF )) = LOAD_BYTE_RET ))
                ;;
            11)
                #echo stb >&2
                parse_mix_arg1
                parse_mix_arg2
                parse_mix_arg3
                store_byte $(( (ARG2 + ARG3) & 0xFFFFFFFF )) $(( ARG1 & 0xFF ))
                ;;
            12)
                #echo ims >&2
                : $(( r$(( ARG1&0xF )) = (r$(( ARG1&0xF )) << 16) & 0xFFFF0000 | (ARG3 << 8) | ARG2 ))
#                parse_register $ARG1
#                register_get $PARSE_REGISTER_RET
#                register_set $PARSE_REGISTER_RET $(( ($REGISTER_GET_RET << 16) & 0xFFFFFFFF | (ARG3 << 8) | ARG2 ))
                ;;
            13)
                #echo ltu >&2
                parse_mix_arg2
                parse_mix_arg3

                # Our comparison is supposed to be unsigned but a POSIX shell
                # only needs to support signed long arithmetic which will be
                # 32 bits on a 32-bit platform. We need to check for signedness
                # manually.
                #
                # Only the first case ever happens on a shell with unsigned
                # 32-bit or signed 64-bit integers because all values will be
                # non-negative.
                #
                # mksh is an example of a shell where this logic is needed
                # because numbers are 32-bit signed even when compiled for
                # 64-bit.
                if [ $ARG2 -ge 0 -a $ARG3 -ge 0 ]; then
                    : $(( LTU = ARG2 < ARG3 ))
                elif [ $ARG2 -ge 0 -a $ARG3 -lt 0 ]; then
                    LTU=1
                elif [ $ARG2 -lt 0 -a $ARG3 -ge 0 ]; then
                    LTU=0
                else
                    : $(( LTU = ARG2 > ARG3 ))
                fi
                #echo ltu $ARG2 $ARG3 '-->' $LTU >&2

                : $(( r$(( ARG1&0xF )) = LTU ))
                ;;
            14)
                #echo jz >&2
                parse_mix_arg1
                if [ 0 -eq $ARG1 ]; then
                    : $(( OFFSET = ((ARG3 << 8) | ARG2) & 0xFFFF ))
                    if [ $OFFSET -ge 32768 ]; then
                        : $(( OFFSET |= 0xFFFF0000 ))
                    fi
                    #echo "jump $OFFSET words" >&2
                    : $(( r15 = (r15 + (OFFSET << 2)) & 0xFFFFFFFF ))
                #else
                #    echo "no jump" >&2
                #    :
                fi
                ;;
            15)
                syscall
                ;;
            *)
                fatal "Invalid opcode: $OPCODE_HEX"
        esac
    done
}

registers_init
files_init
process_init
syscalls_init
args_init "$@"
env_init
program_init "$1"
#        store_word $((0x0100FFFC)) 1  #debugging
run



#
# Directory syscalls
#

# TODO on a diropen syscall, do `ls -1` and store the results in a string. To
# read a directory entry, pass the whole string as arguments to a function, the
# function should `shift` off one entry and store the remaining elements back
# to the string.
