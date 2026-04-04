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
# useful. It does technically "work", if you're okay with waiting months to get
# anything done.
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



# Implementation details:
#
# POSIX shell doesn't have arrays so we use dynamically named variables, like
# _0, _1, _2, etc. for memory and REGISTER_0, REGISTER_1, etc. for registers.
# Memory addresses are stored using variable assignment in arithmetic
# expansions (e.g. `: $(( _$i = $x ))`.)
#
# The VM relies heavily on arithmetic expressions, i.e. `$(( ... ))`, which are
# specified by POSIX. Your shell must support these for this VM to work. Some
# syntax does not work in all shells; for example `$(( ++I ))` works in ksh but
# not dash. We use `$(( I += 1 ))` instead.
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
# When run in bash, dash, zsh and BusyBox, all non-syscall instructions
# *should* be done strictly with builtins (although I haven't really confirmed
# that.)
#
# Syscalls, especially file access syscalls, may use external commands (e.g.
# `ls`, `dd`, etc.)
#
# Shells have wildly different performance. This VM is something like 20x
# faster in BusyBox compared to Bash. The other shells I've tested are
# somewhere in between. Even BusyBox is way too slow to make this work though.
# Under BusyBox, this VM is roughly 10,000x slower than the C89 VM.
#
# I have some ideas for how to potentially improve performance but I doubt I
# could make up the >1000x improvement needed to make this useful. Still, I've
# written stuff below in case I ever want to revisit this.
#
# The main thing needed first is a proper benchmarking suite to test the speed
# of various instructions and syscalls individually so we can get an idea of
# what's slow.
#
# I assume the `eval` calls are a major problem so any way to avoid them could
# make a difference. If we can change functions like `parse_mix`,
# `set_register` and `cache_fetch` (on a hit) to not use `eval` (perhaps using
# large `case` statements instead), it might make a big difference. On the
# other hand large case statements might be even slower (as evidenced by the
# slow hex/sh-alt tools.)
#
# I have some ideas for how to improve cache line performance but none of them
# are good. Cache lines could potentially be decimal strings delimited by IFS,
# or each byte in each cache line could be its own variable. This probably
# wouldn't help though. Since the backing is in hexadecimal you don't
# necessarily save much if you need to convert entire cache lines to and from
# hexadecimal whenever you get a cache miss.



#set -e
#set -vx



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
    # Check the memory address
    check_address $1
    LOAD_BYTE_IX=$(( $1 / 4 ))
    LOAD_BYTE_OFFSET=$(( ($1 & 0x3) * 8 ))

    # Extract the value from the appropriate shell variable and byte offset.
    LOAD_BYTE_RET=$(( (_${LOAD_BYTE_IX} >> LOAD_BYTE_OFFSET) & 0xFF ))
}

# Loads a word from the given address and places it in LOAD_WORD_RET.
load_word() {
    # Check the memory address
    check_address $1
    check_alignment $1
    LOAD_BYTE_IX=$(( $1 / 4 ))

    # The adress is aligned so we know it's not split across 2 shell variables
    LOAD_WORD_RET=$(( _${LOAD_BYTE_IX} & 0xFFFFFFFF ))
}

# Stores a byte at the given address.
store_byte() {
    # Check the memory address
    check_address $1

    LOAD_BYTE_IX=$(( $1 / 4 ))
    LOAD_BYTE_OFFSET=$(( ($1 & 0x3) * 8 ))
    # We clear the byte at the appropriate offset and then set it to the new value.
    LOAD_WORD_RET=$(( _${LOAD_BYTE_IX} & 0xFFFFFFFF ))
    LOAD_WORD_RET=$(( LOAD_WORD_RET & ~(0xFF << LOAD_BYTE_OFFSET) )) # clear the byte
    LOAD_WORD_RET=$(( LOAD_WORD_RET | (($2 & 0xFF) << LOAD_BYTE_OFFSET) )) # set the byte to the new value
    : $(( _${LOAD_BYTE_IX} = LOAD_WORD_RET ))
}

# Stores a word at the given address.
store_word() {
    # Check the memory address
    check_address $1
    check_alignment $1

    LOAD_BYTE_IX=$(( $1 / 4 ))
    # the adress is aligned so we know it's not split across 2 shell variables
    : $(( _${LOAD_BYTE_IX} = $2 & 0xFFFFFFFF ))
}



################################################################
# Registers
################################################################

# Registers are stored as 32-bit signed decimal strings.

registers_init() {
    #echo registers_init >&2
    I=0
    while [ $I -ne 16 ]; do
        : $(( REGISTER_$I = 0 ))
        I=$(( $I + 1 ))
    done
}

register_get() {
    REGISTER_GET_RET=$(( REGISTER_$1 ))
}

register_set() {
#    if [ $2 -gt 2147483647 ]; then
#        REGISTER_SET_VALUE=$(( $2 - 4294967296 ))
#        #echo a $2 $REGISTER_SET_VALUE >&2
#    elif [ $2 -lt -2147483648 ]; then
#        #echo b >&2
#        REGISTER_SET_VALUE=$(( $2 + 4294967296 ))
#    else
#        #echo c >&2
#        REGISTER_SET_VALUE=$2
#    fi
#    #echo register_set $1 $REGISTER_SET_VALUE $(printf %08X $REGISTER_SET_VALUE) >&2
#    : $(( REGISTER_$1 = $REGISTER_SET_VALUE ))

    #echo register_set $1 $2 $(printf %08X $2) >&2
    : $(( REGISTER_$1 = $2 ))
}

registers_print() {
    I=0
    while [ $I -ne 16 ]; do
        register_get $I
        echo REGISTER_$I=$REGISTER_GET_RET >&2
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
    for FILE_READ_BYTE in $FILE_READ_BYTES; do
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

    : $(( FILE_WRITE_END = FILE_WRITE_ADDRESS + FILE_WRITE_COUNT ))
    while [ $FILE_WRITE_ADDRESS -ne $FILE_WRITE_END ]; do
        load_byte $FILE_WRITE_ADDRESS
        printf \\$(printf %o $LOAD_BYTE_RET)
        : $(( FILE_WRITE_ADDRESS += 1 ))
    done | \
        if [ "$FILE_WRITE_NAME" = "/dev/stderr" ]; then
            cat >&2
        else
            dd \
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
file_tell() {
    FILE_TELL_FILENAME="$(eval echo \$FILE_${1}_NAME)"
    : $(( FILE_TELL_STREAM = FILE_${1}_STREAM ))
    : $(( FILE_TELL_OFFSET = FILE_${1}_OFFSET ))

    if [ "$FILE_TELL_FILENAME" = "" ]; then
        fatal "Handle is not open."
    fi

    if [ $FILE_TELL_STREAM -eq 1 ]; then
        # cannot seek stream
        FILE_TELL_RET=$ERROR_UNSUPPORTED
        return
    fi

    FILE_TELL_RET=$FILE_TELL_OFFSET
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
    #syscall_enable 18  # mkdir

    # Write the syscall table address into the process info table
    store_word $(( $PROCESS_INFO_TABLE + 8 )) $SYSCALL_TABLE_ADDRESS
}

# Loads the process info vector and its contents (command-line arguments,
# environment variables, halt code, etc.)
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

# Loads the program into memory and sets the program break.
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
    REGISTER_0=$PROCESS_INFO_TABLE  # r0
    REGISTER_12=$MEMORY_END         # rsp
    REGISTER_14=$CURRENT_ADDRESS    # rpp
    REGISTER_15=$CURRENT_ADDRESS    # rip

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
    load_byte $REGISTER_15
    B0=$LOAD_BYTE_RET
    load_byte $(( REGISTER_15 + 1))
    B1=$LOAD_BYTE_RET
    load_byte $(( REGISTER_15 + 2))
    B2=$LOAD_BYTE_RET
    if ( [ $B0 -eq $((0x23)) ] && [ $B1 -eq $((0x21)) ] ) || \
        ( [ $B0 -eq $((0x52)) ] && [ $B1 -eq $((0x45)) ] && [ $B2 -eq $((0x4D)) ] ); then
        # wrap header found
        : $(( REGISTER_14 += 128 ))
        : $(( REGISTER_15 += 128 ))
    fi
}



################################################################
# Syscalls
################################################################

syscall_open() {
    # Read the filename from memory
    FILENAME=
    I=$REGISTER_0
    while true; do
        load_byte $I
        if [ $LOAD_BYTE_RET -eq 0 ]; then
            break
        fi
        FILENAME="$FILENAME$(printf \\$(printf %o $LOAD_BYTE_RET))"
        I=$(( $I + 1 ))
    done

    # Call open
    MODE=$REGISTER_1
    file_open "$FILENAME" $MODE

    # Put the result in r0
    register_set 0 $FILE_OPEN_RET
}

syscall_close() {
    file_close $REGISTER_0
    register_set 0 0
}

syscall_read() {
    HANDLE=$REGISTER_0
    ADDRESS=$REGISTER_1
    COUNT=$REGISTER_2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid read size."
    fi

    file_read $HANDLE $ADDRESS $COUNT
    register_set 0 $FILE_READ_RET
}

syscall_write() {
    HANDLE=$REGISTER_0
    ADDRESS=$REGISTER_1
    COUNT=$REGISTER_2

    # Safety for 32-bit shells
    if [ $COUNT -lt 0 ]; then
        fatal "Invalid write count."
    fi

    file_write $HANDLE $ADDRESS $COUNT
    register_set 0 $FILE_WRITE_RET
}

syscall_seek() {
    HANDLE=$REGISTER_0
    BASE=$REGISTER_1
    OFFSET_LOW=$REGISTER_2
    OFFSET_HIGH=$REGISTER_3

    # We only support up to 2 GiB file size.
    if [ $OFFSET_HIGH -ne 0 ] || [ $OFFSET_LOW -lt 0 ] || [ $OFFSET_LOW -gt $((0x7FFFFFFF)) ]; then
        register_set 0 $ERROR_GENERIC
        return
    fi

    file_seek $HANDLE $BASE $OFFSET_LOW
    register_set 0 $FILE_SEEK_RET
}

syscall_tell() {
    HANDLE=$REGISTER_0
    POSITION=$REGISTER_1

    file_tell $HANDLE

    # We only support up to 2 GiB file size.
    if [ $FILE_TELL_RET -lt 0 ] || [ $FILE_TELL_RET -gt $((0x7FFFFFFF)) ]; then
        register_set 0 $FILE_TELL_RET
        return
    fi

    store_word $POSITION $FILE_TELL_RET
    store_word $(( POSITION + 4 )) 0
    register_set 0 0
}

syscall_trunc() {
    HANDLE=$REGISTER_0
    SIZE_LOW=$REGISTER_1
    SIZE_HIGH=$REGISTER_2

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

syscall() {
    case $REGISTER_9 in
        0)  # halt
            exit $(( $REGISTER_0 & 0xFF ))
            ;;
        3) syscall_open ;;
        4) syscall_close ;;
        5) syscall_read ;;
        6) syscall_write ;;
        7) syscall_seek ;;
        8) syscall_tell ;;
        9) syscall_trunc ;;
        *)
            fatal "Unhandled syscall: $1"
            ;;
    esac

    # load the top of the stack into the instruction pointer
    load_word $REGISTER_12
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

run() {
    while true; do
        #echo "about to load instruction word at $REGISTER_15"
        load_word $REGISTER_15
        : $(( OPCODE = ($LOAD_WORD_RET >> 0)  & 0xFF ))
        : $(( ARG1   = ($LOAD_WORD_RET >> 8)  & 0xFF ))
        : $(( ARG2   = ($LOAD_WORD_RET >> 16) & 0xFF ))
        : $(( ARG3   = ($LOAD_WORD_RET >> 24) & 0xFF ))
        # for easier debugging, but should be removed for performance
        OPCODE_HEX=$(printf %02X $OPCODE)
        #echo >&2
        #echo "instruction at 0x$(printf %08X $REGISTER_15): $(printf %02X $OPCODE) $(printf %02X $ARG1) $(printf %02X $ARG2) $(printf %02X $ARG3)" >&2
#        if [ $OPCODE = "00" ]; then
#            registers_print
#        fi
        REGISTER_15=$(( $REGISTER_15 + 4 ))

        case $OPCODE_HEX in
            70)
                #echo add >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            71)
                #echo sub >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 - $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            72)
                #echo mul >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 * $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            73)
                #echo div >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 / $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            74)
                #echo and >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 & $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            75)
                #echo or >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 | $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            76)
                #echo shl >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                register_set $PARSE_REGISTER_RET $(( ($ARG2 << $PARSE_MIX_RET) & 0xFFFFFFFF ))
                ;;
            77)
                #echo shru >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                ARG3=$PARSE_MIX_RET
                # We only have a signed shift. We shift down by 1, mask out the
                # high bit, then shift the rest.
                if [ $ARG3 -eq 0 ]; then
                    register_set $PARSE_REGISTER_RET $ARG2
                else
                    register_set $PARSE_REGISTER_RET $(( ((($ARG2 >> 1) & 0x7FFFFFFF) >> ($ARG3 - 1)) ))
                fi
                ;;
            78)
                #echo ldw >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                #echo ldw at $ARG2 + $PARSE_MIX_RET into reg $PARSE_REGISTER_RET >&2
                load_word $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                register_set $PARSE_REGISTER_RET $LOAD_WORD_RET
                ;;
            79)
                #echo stw >&2
                parse_mix $ARG1
                ARG1=$PARSE_MIX_RET
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                store_word $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF )) $ARG1
                ;;
            7A)
                #echo ldb >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                load_byte $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF ))
                register_set $PARSE_REGISTER_RET $LOAD_BYTE_RET
                ;;
            7B)
                #echo stb >&2
                parse_mix $ARG1
                ARG1=$PARSE_MIX_RET
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                store_byte $(( ($ARG2 + $PARSE_MIX_RET) & 0xFFFFFFFF )) $(( $ARG1 & 0xFF ))
                ;;
            7C)
                #echo ims >&2
                parse_register $ARG1
                register_get $PARSE_REGISTER_RET
                register_set $PARSE_REGISTER_RET $(( ($REGISTER_GET_RET << 16) & 0xFFFFFFFF | (ARG3 << 8) | ARG2 ))
                ;;
            7D)
                #echo ltu >&2
                parse_register $ARG1
                parse_mix $ARG2
                ARG2=$PARSE_MIX_RET
                parse_mix $ARG3
                ARG3=$PARSE_MIX_RET

                # Our comparison is supposed to be unsigned but a POSIX shell
                # only needs to support signed long arithmetic which will be
                # 32 bits on a 32-bit platform. We need to check for signedness
                # manually. (These first two cases never happen on a 64-bit
                # shell because values are 64-bit unsigned.)
                #echo $ARG2 $ARG3 >&2
                if [ $ARG2 -ge 0 -a $ARG3 -lt 0 ]; then
                    LTU=0
                elif [ $ARG2 -lt 0 -a $ARG3 -ge 0 ]; then
                    LTU=1
                else
                    LTU=$(( $ARG2 < $ARG3 ))
                fi

                register_set $PARSE_REGISTER_RET $LTU
                ;;
            7E)
                #echo jz >&2
                parse_mix $ARG1
                if [ 0 -eq $PARSE_MIX_RET ]; then
                    OFFSET=$(( (($ARG3 << 8) | ARG2) & 0xFFFF ))
                    if [ $OFFSET -ge 32768 ]; then
                        OFFSET=$(( $OFFSET - 65536 ))
                    fi
                    #echo "jump $OFFSET words" >&2
                    REGISTER_15=$(( $REGISTER_15 + ($OFFSET << 2) ))
                else
                    #echo "no jump" >&2
                    :
                fi
                ;;
            7F)
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
