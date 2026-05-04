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



/**
 * This is a simple-as-possible implementation of the Onramp virtual machine in
 * ANSI C. It implements the minimum necessary system calls for bootstrapping a
 * compiler.
 *
 * If you're trying to port Onramp to an old system that only has a C89
 * compiler, this is probably the best place to start.
 *
 * TODO there's some Windows portability stuff here but it's incomplete. We
 * still need to translate paths from Windows-style to UNIX style.
 */



#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    #define _POSIX_C_SOURCE 200809L
    #define VM_POSIX
    #include <unistd.h>
    #include <sys/stat.h>
    extern char** environ;
#elif defined _WIN32
    #include <direct.h>
    #define getcwd _getcwd
    #define environ _environ
    #define mkdir(path, mode) _mkdir(path)
    extern char** _environ;
#else
    #define NO_ENVIRON
    #define NO_GETCWD
#endif

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Onramp requires an 8-bit char and a 32-bit int or long. */
#ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 199901L
        /* In C99 or later we can just use <stdint.h>. */
        #include <stdint.h>
        #define VM_HAVE_C99_TYPES
    #endif
#endif
#ifndef VM_HAVE_C99_TYPES
    /* Otherwise assume int is 32 bits. These are checked in main(). */
    typedef signed char int8_t;
    typedef unsigned char uint8_t;
    typedef signed int int32_t;
    typedef unsigned int uint32_t;
#endif
#undef VM_HAVE_C99_TYPES

#ifndef UINT32_MAX
    #define UINT32_MAX 0xFFFFFFFFu
#endif



/*
 * Globals
 */

#define VM_MEMORY_SIZE (16 * 1024 * 1024) /* 16 MB */
#define VM_MAX_FILES 16u
#define VM_MAX_DIRECTORIES 16u

static uint32_t vm_registers[16];
static uint8_t vm_memory[VM_MEMORY_SIZE];
static FILE* vm_files[VM_MAX_FILES];
/*static uint32_t vm_directories[VM_MAX_DIRECTORIES];*/

/* array indices of named registers */
#define VM_RSP 0xC  /* stack pointer */
#define VM_RFP 0xD  /* frame pointer */
#define VM_RPP 0xE  /* program pointer */
#define VM_RIP 0xF  /* instruction pointer */

/* errors */
#define VM_ERROR_GENERIC      0xFFFFFFFF
#define VM_ERROR_PATH         0xFFFFFFFE
#define VM_ERROR_IO           0xFFFFFFFD
#define VM_ERROR_UNSUPPORTED  0xFFFFFFFC
#define VM_ERROR_TRY_LATER    0xFFFFFFFB
#define VM_ERROR_END_OF_FILE  0xFFFFFFFA
#define VM_ERROR_OVERFLOW     0xFFFFFFF9
#define VM_ERROR_IN_USE       0xFFFFFFF8

/* The number of entries in the syscall table (not the number we have actually
 * implemented) */
#define VM_SYSCALL_COUNT 25

static uint8_t vm_load_u8(uint32_t addr);

typedef uint32_t syscall_fn_t(void);
static syscall_fn_t* vm_syscall_table[VM_SYSCALL_COUNT];



/*
 * Utilities and Error Checking
 */

#if 0
static void vm_hexdump(const char* p, int32_t count) {
    int32_t i;
    for (i = 0; i < count; ++i) {
        printf("0x%08x -- 0x%02X %c %i\n",
                i,
                p[i],
                (p[i] >= 32 && p[i] <= 126) ? p[i] : '.',
                p[i]);
    }
}
#endif

static void vm_panic(const char* msg) {
    fprintf(stderr, "VM ERROR: %s\n", msg);
    exit(125);
}



/*
 * Memory Access
 */

static uint32_t vm_load_u32(uint32_t addr) {
    /* VM memory is little-endian. */
    return (uint32_t)vm_memory[addr] |
            ((uint32_t)vm_memory[addr + 1] << 8) |
            ((uint32_t)vm_memory[addr + 2] << 16) |
            ((uint32_t)vm_memory[addr + 3] << 24);
}

static void vm_store_u32(uint32_t addr, uint32_t value) {
    /* VM memory is little-endian. */
    vm_memory[addr]     = (uint8_t)value;
    vm_memory[addr + 1] = (uint8_t)(value >> 8);
    vm_memory[addr + 2] = (uint8_t)(value >> 16);
    vm_memory[addr + 3] = (uint8_t)(value >> 24);
}

static uint8_t vm_load_u8(uint32_t addr) {
    return vm_memory[addr];
}

static void vm_store_u8(uint32_t addr, uint8_t value) {
    vm_memory[addr] = value;
}

static size_t vm_store_string(uint32_t addr, const char* str) {
    uint32_t size = (uint32_t)strlen(str) + 1;
    memcpy(vm_memory + addr, str, size);
    return addr + (uint32_t)size;
}

static uint32_t vm_store_string_array(uint32_t addr, char** strings) {
    uint32_t count;
    uint32_t array;

    /* count strings */
    count = 0;
    for (; strings[count] != NULL; ++count) {}

    /* make space for array */
    array = addr;
    addr += (count + 1) * 4;

    /* load strings */
    for (; *strings; ++strings) {
        vm_store_u32(array, addr);
        array += 4;
        addr = vm_store_string(addr, *strings);
    }
    vm_store_u32(array, 0);

    /* align address */
    addr = (addr + 0x3u) & ~0x3u;
    return addr;

}



/*
 * Miscellaneous
 */

static uint32_t vm_parse_mix(uint8_t b) {
    if (b <= 0x7Fu)
        return b;
    if (b >= 0x90u)
        return (uint32_t)b | 0xFFFFFF00u;
    if ((b & 0xF0) == 0x80)
        return vm_registers[b & 0x0F];
    vm_panic("Invalid value for mix-type argument");
    return 0;
}

static uint8_t vm_parse_register(uint8_t b) {
    if ((b & 0xF0) != 0x80)
        vm_panic("Invalid register");
    return b & 0x0F;
}

static FILE* vm_file(uint32_t handle) {
    return vm_files[handle];
}



/*
 * Initialization
 */

static void vm_load_program(uint32_t* /*in-out*/ start, uint32_t* /*out*/ end, const char* filename) {
    FILE* file;
    uint32_t addr;

    /* Read the entire program into memory */
    file = fopen(filename, "rb");
    if (file == NULL) {
        vm_panic("Couldn't open program");
    }
    addr = *start;
    for (;;) {
        size_t ret = fread(vm_memory + addr, 1, VM_MEMORY_SIZE - addr, file);
        if (ret == 0) {
            if (feof(file))
                break;
            vm_panic("Error reading program!");
        }
        addr += ret;
    }
    fclose(file);

    /* Align the end address */
    addr = (addr + 0x3u) & ~0x3u;
    *end = addr;

    /* Make sure there's still at least some room for heap and stack */
    if (VM_MEMORY_SIZE - addr < 32 * 1024) {
        vm_panic("Program is too big.");
    }

    /* Check for a #! or REM prefix */
    if ((vm_load_u8(*start) == '#' && vm_load_u8(*start + 1) == '!') ||
            (vm_load_u8(*start) == 'R' &&
             vm_load_u8(*start + 1) == 'E' &&
             vm_load_u8(*start + 2) == 'M'))
    {
        *start += 128;
    }

    /* Check the format indicator */
    if (vm_load_u32(*start) != 0x726E4F7E ||
            vm_load_u32(*start + 4) != 0x706D617E ||
            vm_load_u32(*start + 8) != 0x2020207E)
    {
        /*printf("%x\n", vm_load_u32(start));*/
        fprintf(stderr, "WARNING: Program does not start with \"~Onr~amp~   \" format indicator.\n");
    }
}

static void vm_init(int argc, char** argv) {
    const char* filename = NULL;
    int i;
    uint32_t address, process_info_address,
             syscall_address, syscall_table_address,
             program_start, program_break;
    char** env = 0;
    char* cwd = 0;
    char cwd_buffer[512];

    if (argc <= 1) {
        fputs("ERROR: No program filename specified.\n", stderr);
        exit(125);
    }
    filename = argv[1];

    /* Our process info table starts after zero to make sure it isn't
     * interpreted as a null pointer. */
    address = 4;

    /* Reserve space for process info table */
    process_info_address = address;
    address += 48;

    /* configure process info table */
    vm_store_u32(process_info_address + 0, 4); /* version */
    vm_store_u32(process_info_address + 12, 0); /* stdin */
    vm_store_u32(process_info_address + 16, 1); /* stdout */
    vm_store_u32(process_info_address + 20, 2); /* stderr */
    vm_store_u32(process_info_address + 40, 0); /* minor version */
    vm_store_u32(process_info_address + 44, 0); /* additional memory regions */

    /* make an instruction with opcode 0x7F to use for syscalls */
    syscall_address = address;
    vm_store_u8(syscall_address, 0x7F);
    address += 4;

    /* build syscall table */
    syscall_table_address = address;
    vm_store_u32(process_info_address + 8, syscall_table_address);
    for (i = 0; i < VM_SYSCALL_COUNT; ++i) {
        /* If we implement this syscall, pass the address of our 0x7F opcode as
         * the syscall address and the syscall number as the context. */
        if (vm_syscall_table[i] != NULL) {
            vm_store_u32(syscall_table_address + i * 8, syscall_address);
            vm_store_u32(syscall_table_address + i * 8 + 4, i);
        } else {
            vm_store_u32(syscall_table_address + i * 8, 0);
        }
        address += 8;
    }

    /* args */
    vm_store_u32(process_info_address + 24, address);
    address = vm_store_string_array(address, argv + 1); /* skip vm name */

    /* environment variables */
    #ifndef NO_ENVIRON
    env = environ;
    #endif
    if (env) {
        vm_store_u32(process_info_address + 28, address);
        address = vm_store_string_array(address, environ);
    } else {
        vm_store_u32(process_info_address + 28, 0);
    }

    /* working directory */
    #ifndef NO_GETCWD
    cwd = getcwd(cwd_buffer, sizeof(cwd_buffer));
    #endif
    if (cwd) {
        vm_store_u32(process_info_address + 32, address);
        address = vm_store_string(address, cwd);
        address = (address + 0x3u) & ~0x3u; /* align address */
    } else {
        vm_store_u32(process_info_address + 32, 0);
    }

    /* capabilities */
    vm_store_u32(process_info_address + 36,
            /* We'll assume the terminal echoes, blocks, and buffers lines.
             * Even if it doesn't, the Onramp libc will handle it gracefully,
             * except that it will reject attempts to turn these off. This is
             * the safest option for POSIX systems. */
            7);

    /* files */
    vm_files[0] = stdin;
    vm_files[1] = stdout;
    vm_files[2] = stderr;

    /* Load the program */
    program_start = address;
    vm_load_program(&program_start, &program_break, filename);

    /* Setup registers */
    vm_registers[0] = process_info_address;
    vm_registers[VM_RSP] = VM_MEMORY_SIZE;
    vm_registers[VM_RPP] = program_start;
    vm_registers[VM_RIP] = program_start;

    /* Store the break address in the process info table */
    vm_store_u32(process_info_address + 4, program_break);
}



/*
 * System Calls
 */

static uint32_t vm_exit(void) {
    exit(vm_registers[0]);
}

static uint32_t vm_open(void) {
    uint32_t path_addr = vm_registers[0];
    uint32_t mode = vm_registers[1];
    FILE* file;

    const char* path;
    size_t handle;

    path = (const char*)vm_memory + path_addr;
    /*fprintf(stderr, "open %s %u\n", path, mode);*/

    /* find a free handle (not the standard streams 0,1,2) */
    for (handle = 3; handle < VM_MAX_FILES &&
            vm_files[handle] != NULL; ++handle) {}
    if (handle == VM_MAX_FILES) {
        /* too many open files */
        return VM_ERROR_GENERIC;
    }

    if (mode) {
        /* Try to open the existing file read/write. If it fails, it's probably
         * because it doesn't exist, so try opening for writing to create it.
         * (We're not concerned with race conditions in Onramp.) */
        file = fopen(path, "r+b");
        if (file == NULL) {
            file = fopen(path, "w+b");
        }
    } else {
        file = fopen(path, "rb");
    }

    if (file == NULL) {
        /* TODO there may be other reasons it failed. Need to implement v4
         * error codes. */
        return VM_ERROR_PATH;
    }
    vm_files[handle] = file;

    return handle;
}

static uint32_t vm_close(void) {
    uint32_t handle = vm_registers[0];
    fclose(vm_files[handle]);
    vm_files[handle] = NULL;
    return 0;
}

static uint32_t vm_read(void) {
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];
    FILE* file = vm_file(vm_registers[0]);
    size_t ret = fread(vm_memory + addr, 1, count, file);
    if (ret == 0) {
        return feof(file) ? VM_ERROR_END_OF_FILE : VM_ERROR_GENERIC;
    }
    return (uint32_t)ret;
}

static uint32_t vm_write(void) {
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];
    FILE* file = vm_file(vm_registers[0]);
    size_t ret = fwrite(vm_memory + addr, 1, count, file);
    if (ret == count) {
        return count;
    }
    return VM_ERROR_GENERIC;
}

static uint32_t vm_seek(void) {
    /*
     * We don't know how long `long` or `off_t` are. If they're only 32 bits we
     * won't have enough space for the high bits. We try anyway; we just won't
     * support files larger than 2 GB otherwise.
     *
     * In case they are only 32 bits, we have to shift twice since a shift by
     * the word size is undefined behaviour.
     */
    FILE* file = vm_file(vm_registers[0]);
    long offset = (long)vm_registers[1] | (((long)vm_registers[2] << 16) << 16);
    int ret = fseek(file, offset, SEEK_SET);
    return ret ? VM_ERROR_GENERIC : 0;
}

static uint32_t vm_size(void) {
    FILE* file = vm_file(vm_registers[0]);
    uint32_t addr = vm_registers[1];
    unsigned long usize;
    long pos, size;
    int ret;

    /* We get the size by seeking to the end, then seeking back. We need to
     * store the current position so we can restore it afterwards. */
    pos = ftell(file);
    if (pos < 0) {
        return VM_ERROR_UNSUPPORTED;
    }

    ret = fseek(file, 0, SEEK_END);
    if (ret != 0) {
        return VM_ERROR_GENERIC;
    }

    size = ftell(file);
    if (size < 0) {
        /* Can't get the size. At least try to seek back to where we were.
         * We already have an error so ignore the return value. */
        (void)fseek(file, pos, SEEK_SET);
        return VM_ERROR_GENERIC;
    }

    if (0 != fseek(file, pos, SEEK_SET)) {
        /* Can't seek back to where we were! TODO this leaves the file in a
         * broken state. We should probably do something else here, maybe panic
         * (and also panic if the previous fseek fails) */
        return VM_ERROR_GENERIC;
    }

    /* As with vm_seek() we shift twice in case `long` or `off_t` is only
     * 32 bits. We convert to unsigned first to get an unsigned shift. */
    usize = (unsigned long)size;
    vm_store_u32(addr, (uint32_t)usize);
    vm_store_u32(addr + 4, (uint32_t)((usize >> 16) >> 16));
    return 0;
}

#ifdef VM_POSIX
static uint32_t vm_trunc(void) {
    /* On POSIX systems we call ftruncate(). */
    uint32_t size_low = vm_registers[1];
    uint32_t size_high = vm_registers[2];
    FILE* file = vm_file(vm_registers[0]);
    int fd = fileno(file);
    /* Again we shift twice in case off_t is 32 bits. */
    off_t upos = (off_t)size_low | (((off_t)size_high << 16) << 16);
    int ret;
    fflush(file);
    ret = ftruncate(fd, upos);
    return ret ? VM_ERROR_GENERIC : 0;
}
#elif
static uint32_t vm_trunc(void) {
    /* On Windows we have _chsize(). There is also _chsize_s() which is
     * 64-bit but our fseek()/ftell() functions aren't currently using
     * corresponding 64-bit functions so right now there's no point. */
    uint32_t size_low = vm_registers[1];
    uint32_t size_high = vm_registers[2];
    FILE* file = vm_file(vm_registers[0]);
    unsigned long upos = (unsigned long)size_low |
            (((unsigned long)size_high << 16) << 16);
    int ret;
    fflush(file);
    ret = _chsize(fileno(file), upos);
    return ret ? VM_ERROR_GENERIC : 0;
}
#else
#define vm_trunc NULL
#endif

/* The remove() function is standard C. It can delete directories as long as
 * they are empty. */
static uint32_t vm_delete(void) {
    const char* path = (const char*)vm_memory + vm_registers[0];
    return remove(path) ? VM_ERROR_GENERIC : 0;
}

#ifdef VM_POSIX
static uint32_t vm_chmod(void) {
    /* There is nothing like chmod() in standard C. It's only relevant for
     * better integration into UNIX systems. */
    uint32_t path_addr = vm_registers[0];
    uint32_t mode = vm_registers[1];
    const char* path = (const char*)vm_memory + path_addr;
    return chmod(path, mode) ? VM_ERROR_GENERIC : 0;
}
#else
#define vm_chmod NULL
#endif

#if defined(VM_POSIX) || defined(_WIN32)
static uint32_t vm_mkdir(void) {
    /* Standard C doesn't have mkdir but POSIX does. */
    const char* path = (const char*)vm_memory + vm_registers[0];
    return mkdir(path, 0755) ? VM_ERROR_GENERIC : 0;
}
#else
#define vm_mkdir NULL
#endif

static syscall_fn_t* vm_syscall_table[VM_SYSCALL_COUNT] = {
    vm_exit,
    NULL, /* panic */
    NULL, /* time */
    vm_open,
    vm_close,
    vm_read,
    vm_write,
    vm_seek,
    vm_size,
    vm_trunc, /* may be NULL */
    NULL, /* (unused) */
    NULL, /* (unused) */
    NULL, /* dirent */
    NULL, /* stat */
    NULL, /* rename */
    NULL, /* (unused) */
    vm_delete, /* delete */
    vm_chmod, /* may be NULL */
    vm_mkdir, /* may be NULL */
    NULL, /* (unused) */
    NULL, /* (unused) */
    NULL, /* (unused) */
    NULL, /* debug */
    NULL, /* alloc */
    NULL, /* free */
};

static void vm_sys(void) {
    uint32_t syscall = vm_registers[9];
    if (syscall >= VM_SYSCALL_COUNT) {
        vm_panic("Invalid syscall number.");
    }
    vm_registers[0] = vm_syscall_table[syscall]();
    vm_registers[VM_RIP] = vm_load_u32(vm_registers[VM_RSP]);
}



/*
 * Main Loop
 */

static void vm_run(void) {
    uint32_t rip;
    uint8_t opcode, arg1, arg2, arg3;
    uint32_t* reg;
    uint32_t mix1, mix2;
next:

    /* Parse instruction */
    rip = vm_registers[VM_RIP];
    opcode = vm_memory[rip];
    arg1 = vm_memory[rip + 1];
    arg2 = vm_memory[rip + 2];
    arg3 = vm_memory[rip + 3];
    /*printf("Running instruction %02x%02x%02x%02x at %u\n",opcode, arg1, arg2,arg3,vm_registers[VM_RIP]);*/
    vm_registers[VM_RIP] += 4;

    /* Handle the opcodes with non-typical arguments first. */
    switch (opcode) {
        case 0x79: /* stw */
            vm_store_u32(vm_parse_mix(arg2) + vm_parse_mix(arg3), vm_parse_mix(arg1));
            goto next;
        case 0x7B: /* stb */
            vm_store_u8(vm_parse_mix(arg2) + vm_parse_mix(arg3), (uint8_t)vm_parse_mix(arg1));
            goto next;
        case 0x7C: { /* ims */
            size_t dest = vm_parse_register(arg1);
            vm_registers[dest] <<= 16;
            vm_registers[dest] |= (uint32_t)arg2 | ((uint32_t)arg3 << 8);
            goto next;
        }
        case 0x7E: /* jz */
            if (0 == vm_parse_mix(arg1))
                vm_registers[VM_RIP] += ((uint32_t)arg2 | (uint32_t)((int32_t)(int8_t)arg3 << 8)) << 2;
            goto next;
        case 0x7F: /* special internal syscall opcode */
            vm_sys();
            goto next;

        default: break;
    }

    /* The remaining opcodes all place the result of an operation on two
     * mix-type arguments into a destination register. */
    reg = vm_registers + vm_parse_register(arg1);
    mix1 = vm_parse_mix(arg2);
    mix2 = vm_parse_mix(arg3);
    switch (opcode) {
        case 0x70:   *reg = mix1 + mix2;   goto next;  /* add */
        case 0x71:   *reg = mix1 - mix2;   goto next;  /* sub */
        case 0x72:   *reg = mix1 * mix2;   goto next;  /* mul */
        case 0x73:   *reg = mix1 / mix2;   goto next;  /* div */
        case 0x74:   *reg = mix1 & mix2;   goto next;  /* and */
        case 0x75:   *reg = mix1 | mix2;   goto next;  /* or */
        case 0x76:   *reg = mix1 << mix2;  goto next;  /* shl */
        case 0x77:   *reg = mix1 >> mix2;  goto next;  /* shru */
        case 0x78:   *reg = vm_load_u32(mix1 + mix2);   goto next;  /* ldw */
        case 0x7A:   *reg = vm_load_u8 (mix1 + mix2);   goto next;  /* ldb */
        case 0x7D:   *reg = mix1 < mix2;   goto next;  /* ltu */
        default:
            break;
    }

    vm_panic("Invalid instruction");
}

int main(int argc, char** argv) {
    if (CHAR_BIT != 8) {
        fputs("ERROR: CHAR_BIT is not 8. An 8-bit char is required.\n", stderr);
        exit(1);
    }
    if (sizeof(uint32_t) != 4) {
        fputs("ERROR: A 32-bit integer type is required.\n", stderr);
        exit(1);
    }
    vm_init(argc, argv);
    vm_run();
    return 1;
}
