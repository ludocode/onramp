/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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
 * This is a simple implementation of the Onramp virtual machine in ANSI C.
 *
 * It does not load debug info or do any debugging. It otherwise performs all
 * required checks and many optional checks and implements all system calls.
 *
 * If you're trying to port Onramp to an old system that only has a C89
 * compiler, this is probably the best place to start. There's a good chance
 * you'll need to modify this; in particular, there is currently no
 * implementation of the ftrunc syscall in standard C.
 *
 * The VM attempts to enable raw input mode on POSIX platforms. If this is
 * causing problems on older UNIX you can disable the detection of VM_POSIX
 * below.
 *
 * TODO there's some Windows portability stuff here but it's incomplete. We
 * still need to translate paths from Windows-style to UNIX style.
 */



/*
 * Portability
 */

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    /* We currently rely on ftruncate(), clock_gettime(), environ, and terminal
     * settings on POSIX systems. */
    #define VM_POSIX
    #define _POSIX_C_SOURCE 200809L
    #include <errno.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <termios.h>
    #include <unistd.h>
    extern char** environ;
#endif

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    extern char** _environ;
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


#ifdef _WIN32
    #define getcwd _getcwd
    #define environ _environ
#endif



/*
 * Globals
 */

#define VM_MEMORY_SIZE (16 * 1024 * 1024) /* 16 MB */
#define VM_MAX_FILES 16
#define VM_MAX_DIRECTORIES 16

/* We use platform-specific file APIs where possible. This allows us to provide
 * features that aren't possible in standard C (such as non-blocking input.) */
#ifdef VM_POSIX
    typedef int file_t;
    #define INVALID_FILE -1
#elif defined _WIN32
    #error TODO windows file API
#else
    typedef FILE* file_t;
    #define INVALID_FILE NULL
#endif

static uint32_t vm_registers[16];
static uint8_t vm_memory[VM_MEMORY_SIZE];
static file_t vm_files[VM_MAX_FILES];
/*static uint32_t vm_directories[VM_MAX_DIRECTORIES];*/

/* array indices of named registers */
#define VM_RSP 0xC  /* stack pointer */
#define VM_RFP 0xD  /* frame pointer */
#define VM_RPP 0xE  /* program pointer */
#define VM_RIP 0xF  /* instruction pointer */

/* errors */
#define VM_ERR_GENERIC     0xFFFFFFFF
#define VM_ERR_PATH        0xFFFFFFFE
#define VM_ERR_IO          0xFFFFFFFD
#define VM_ERR_UNSUPPORTED 0xFFFFFFFC

static uint8_t vm_load_u8(uint32_t addr);



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
    #if 0
        #ifdef __GNUC__
            __builtin_trap();
        #endif
    #endif
    exit(125);
}

static void usage(const char* command) {
    fprintf(stderr, "Usage: %s [vm options] <program> [program options]\n", command);
    fputs("\n", stderr);
    /* TODO no options are currently supported.
    fprintf(stderr, "VM options:\n");
    fprintf(stderr, "    -e NAME=VAR       define environment variable\n"); // TODO probably don't need this since we now forward env vars from the environment
    */
    exit(125);
}



/*
 * Memory Checks
 */

#define vm_check(expr, msg) (!(expr) ? vm_panic(msg) : (void)0)

#define vm_check_aligned(addr) \
    vm_check(((addr) & 3) == 0, "Misaligned address")

/* Our virtual memory starts at 0 but we disable access to the first word to
 * prevent null pointer dereferences. */
#define vm_check_valid(addr) \
    vm_check((addr) >= 4 && (addr) < VM_MEMORY_SIZE, "Address out of bounds")

#define vm_check_file(handle) \
    vm_check((uint32_t)(handle) < VM_MAX_FILES && vm_files[(handle)] != INVALID_FILE, \
            "Invalid file descriptor")

#define vm_check_directory(dd) \
    vm_check((uint32_t)(dd) < VM_MAX_FILES && vm_directories[(dd)] != INVALID_FILE, \
            "Invalid directory descriptor")

static void vm_check_string(uint32_t addr) {
    while (vm_load_u8(addr++) != '\0') {}
}

static void vm_check_buffer(uint32_t addr, uint32_t size) {
    vm_check(size > 0, "Invalid size of zero for syscall buffer");
    vm_check_valid(addr);
    vm_check_valid(addr + size - 1);
}



/*
 * Memory Access
 */

static uint32_t vm_load_u32(uint32_t addr) {
    vm_check_aligned(addr);
    vm_check_valid(addr);
    /* VM memory is little-endian. */
    return (uint32_t)vm_memory[addr] |
            ((uint32_t)vm_memory[addr + 1] << 8) |
            ((uint32_t)vm_memory[addr + 2] << 16) |
            ((uint32_t)vm_memory[addr + 3] << 24);
}

static void vm_store_u32(uint32_t addr, uint32_t value) {
    vm_check_aligned(addr);
    vm_check_valid(addr);
    /* VM memory is little-endian. */
    vm_memory[addr]     = (uint8_t)value;
    vm_memory[addr + 1] = (uint8_t)(value >> 8);
    vm_memory[addr + 2] = (uint8_t)(value >> 16);
    vm_memory[addr + 3] = (uint8_t)(value >> 24);
}

static uint8_t vm_load_u8(uint32_t addr) {
    vm_check_valid(addr);
    return vm_memory[addr];
}

static void vm_store_u8(uint32_t addr, uint8_t value) {
    vm_check_valid(addr);
    vm_memory[addr] = value;
}

static size_t vm_store_string(uint32_t addr, const char* str) {
    uint32_t size = (uint32_t)strlen(str) + 1;
    vm_check_buffer(addr, size);
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
    vm_check_buffer(array, addr - array);

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

static file_t vm_file(uint32_t handle) {
    vm_check_file(handle);
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

#ifdef VM_POSIX
static struct termios saved_termios;

static void io_teardown(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
}

static void signal_handler(int signal) {
    io_teardown();
    _Exit(128 + signal);
}

static void io_setup(void) {
    /* Save the terminal state first before setting our cleanup callbacks */
    tcgetattr(STDIN_FILENO, &saved_termios);

    /* Restore terminal state on exit */
    atexit(io_teardown);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Set unbuffered, non-blocking input */
    setvbuf(stdin, NULL, _IONBF, BUFSIZ);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

    /* Set non-canonical input, no input echo */
    {
        struct termios termios = saved_termios;
        termios.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &termios);
    }
}
#elif defined _WIN32
static void io_setup(void) {
    /* TODO set windows to raw input mode */
}
#else
static void io_setup(void) {
    /* There isn't much we can do in standard C to set up raw input mode. We
     * can at least ask the libc not to buffer input. */
    setvbuf(stdin, NULL, _IONBF, BUFSIZ);
}
#endif

static void vm_init(int argc, char** argv) {
    const char* filename = NULL;
    int i;
    uint32_t address, process_info_address, halt_address, program_start, program_break;
    char** env = 0;
    char* cwd = 0;
    char cwd_buffer[256];

    io_setup();

    for (i = 1; i < argc; ++i) {
        /* TODO parse args */

        if (filename != NULL) {
            fprintf(stderr, "ERROR: Only one program image can be specified.");
            usage(argv[0]);
        }
        filename = argv[i];
        break;
    }

    if (filename == NULL) {
        fputs("ERROR: No program filename specified.\n", stderr);
        usage(argv[0]);
    }

    /* Our process info table starts after zero to make sure it isn't
     * interpreted as a null pointer. */
    address = 4;

    /* Reserve space for process info table */
    process_info_address = address;
    address += 40;

    /* write halt instruction */
    halt_address = address;
    vm_store_u32(address, 0x0000007f);
    address += 4;

    /* configure process info table */
    vm_store_u32(process_info_address + 0, 1); /* version */
    vm_store_u32(process_info_address + 8, halt_address);
    vm_store_u32(process_info_address + 12, 0); /* stdin */
    vm_store_u32(process_info_address + 16, 1); /* stdout */
    vm_store_u32(process_info_address + 20, 2); /* stderr */

    /* args */
    vm_store_u32(process_info_address + 24, address);
    address = vm_store_string_array(address, argv + 1); /* skip vm name */

    /* environment variables */
    #if defined(_WIN32) || defined(VM_POSIX)
        env = environ;
    #endif
    if (env) {
        vm_store_u32(process_info_address + 28, address);
        address = vm_store_string_array(address, environ);
    } else {
        vm_store_u32(process_info_address + 28, 0);
    }

    /* working directory */
    #if defined(_WIN32) || defined(VM_POSIX)
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
            #ifdef VM_POSIX
            0 /* no echo, non-blocking, non-canonical */
            #elif defined _WIN32
            7 /* TODO set windows to raw input mode */
            #else
            /* We're not sure of the platform but we'll assume the terminal
             * echoes, blocks, and buffers lines. Even if it doesn't, the
             * Onramp libc will handle it gracefully, except that it will
             * reject attempts to turn these off. This is the safest option. */
            7
            #endif
            );

    /* files */
    #ifdef VM_POSIX
        vm_files[0] = STDIN_FILENO;
        vm_files[1] = STDOUT_FILENO;
        vm_files[2] = STDERR_FILENO;
        {
            size_t i;
            for (i = 3; i < VM_MAX_FILES; ++i) {
                vm_files[i] = INVALID_FILE;
            }
        }
    #else
        vm_files[0] = stdin;
        vm_files[1] = stdout;
        vm_files[2] = stderr;
    #endif

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

static void vm_halt(void) {
    exit(vm_registers[0]);
}

static void vm_time(void) {
    uint32_t addr = vm_registers[0];

    /* We can get a timespec under POSIX systems or C11. This matches what
     * Onramp expects. (We assume TIME_UTC is relative to the UNIX timestamp.) */
    #if defined(VM_POSIX) || __STDC_VERSION__ >= 201112L
    struct timespec ts;
    if (
        #if defined(VM_POSIX)
        0 == clock_gettime(CLOCK_REALTIME, &ts)
        #else
        TIME_UTC == timespec_get(&ts, TIME_UTC)
        #endif
    ) {
        vm_store_u32(addr, (uint32_t)ts.tv_sec);
        vm_store_u32(addr + 4, sizeof(ts.tv_sec) > 4 ? (uint32_t)(ts.tv_sec >> 32) : 0);
        vm_store_u32(addr + 8, (uint32_t)ts.tv_nsec);
    }
    #else

    /* If we have only C89 and not POSIX, we have to approximate it using
     * time() and clock(). We start with time() and then add the clock() delta
     * to it on each call. We're careful to avoid 64-bit integer math here in
     * case we only have 32 bits. */
    static int initialized = 0;
    static time_t secs;
    static clock_t nanos = 0;
    static clock_t last_clock;

    clock_t current_clock = clock();
    clock_t delta = current_clock - last_clock;
    clock_t delta_secs = delta / CLOCKS_PER_SEC;
    clock_t delta_remainder = delta % CLOCKS_PER_SEC;

    if (!initialized) {
        initialized = 1;
        secs = time(NULL);
        last_clock = current_clock;
    }

    secs += delta_secs;
    nanos += (uint32_t)(delta_remainder * (1000000000 / CLOCKS_PER_SEC));
    if (nanos > 1000000000) {
        nanos -= 1000000000;
        ++secs;
    }
    last_clock = current_clock;

    /* time_t is allowed to be a floating point type on non-POSIX systems. The
     * implementation might have only 64-bit doubles or only 64-bit integer
     * math, not both. We need to do this somewhat roundabout calculation to
     * divide by 2^32 that works in both cases. */
    vm_store_u32(addr, (uint32_t)secs);
    vm_store_u32(addr + 4, sizeof(secs) > 4 ? (uint32_t)(secs / (2 * (time_t)((uint32_t)1 << 31))) : 0);
    vm_store_u32(addr + 8, (uint32_t)nanos);
    #endif

    vm_registers[0] = 0;
}

static void vm_fopen(void) {
    uint32_t path_addr = vm_registers[0];
    uint32_t mode = vm_registers[1];

    const char* path;
    uint32_t handle;
    size_t i;

    vm_check_string(path_addr);
    path = (const char*)vm_memory + path_addr;
    /*fprintf(stderr, "open %s %u\n", path, mode);*/

    /* find a free handle (not the standard streams 0,1,2) */
    handle = UINT32_MAX;
    for (i = 3; i < (size_t)VM_MAX_FILES; ++i) {
        if (vm_files[i] == INVALID_FILE) {
            handle = i;
            break;
        }
    }
    vm_check(handle != UINT32_MAX, "No free file descriptors"); /* TODO this should not be a fatal error */

    /* open it */
    #ifdef VM_POSIX
        vm_files[handle] = open(path, mode ? (O_CREAT | O_APPEND | O_RDWR) : O_RDONLY, 0644);
    #elif defined _WIN32
        #error TODO _WIN32 fopen
    #else
        vm_files[handle] = fopen(path, mode ? "a+b" : "rb");
    #endif
    if (vm_files[handle] == INVALID_FILE) {
        vm_registers[0] = VM_ERR_PATH;
        return;
    }

    /* if writeable, seek to the beginning */
    if (mode) {
        #ifdef VM_POSIX
        lseek
        #else
        fseek
        #endif
            (vm_files[handle], 0, SEEK_SET);
    }

    vm_registers[0] = handle;
}

static void vm_fclose(void) {
    uint32_t handle = vm_registers[0];
    vm_check_file(handle);
    vm_check(handle > 2, "Cannot close standard streams.");

    #ifdef VM_POSIX
        close(vm_files[handle]);
    #elif defined _WIN32
        #error TODO _WIN32 fclose
    #else
        fclose(vm_files[handle]);
    #endif

    vm_files[handle] = INVALID_FILE;
    vm_registers[0] = 0;
}

static void vm_fread(void) {
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];

    vm_check_buffer(addr, count);

    #ifdef VM_POSIX
    {
        int fd = vm_file(vm_registers[0]);
        ssize_t ret = read(fd, vm_memory + addr, count);
        if (ret < 0) {
            if (errno == EWOULDBLOCK) {
                ret = 0;
            } else {
                /* TODO handle closed input stream gracefully */
                ret = VM_ERR_IO;
            }
        }
        vm_registers[0] = (uint32_t)ret;
    }

    #elif defined _WIN32
        #error TODO _WIN32 fread

    #else
    {
        FILE* file = vm_file(vm_registers[0]);
        size_t ret = fread(vm_memory + addr, 1, count, file);
        if (ret == 0 && !feof(file)) {
            vm_registers[0] = VM_ERR_IO;
            return;
        }
        vm_registers[0] = (uint32_t)ret;
    }
    #endif
}

static void vm_fwrite(void) {
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];

    vm_check_buffer(addr, count);

    #ifdef VM_POSIX
    {
        int fd = vm_file(vm_registers[0]);
        ssize_t ret = write(fd, vm_memory + addr, count);
        if (ret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                ret = 0;
            } else {
                ret = VM_ERR_IO;
            }
        }
        vm_registers[0] = (uint32_t)ret;
    }

    #elif defined _WIN32
        #error TODO _WIN32 fwrite

    #else
    {
        FILE* file = vm_file(vm_registers[0]);
        size_t ret = fwrite(vm_memory + addr, 1, count, file);
        if (ret == count) {
            fflush(file);
            vm_registers[0] = count;
        } else {
            vm_registers[0] = VM_ERR_IO;
        }
    }
    #endif
}

static void vm_fseek(void) {
    uint32_t base = vm_registers[1]; /* TODO check valid */

    /*
     * We don't know how long `long` or `off_t` are. If they're only 32 bits we
     * won't have enough space for the high bits. We try anyway; we just won't
     * support files larger than 2 GB otherwise.
     *
     * In case they are only 32 bits, we have to shift twice since a shift by
     * the word size is undefined behaviour.
     */

    #ifdef VM_POSIX
    {
        /* TODO we should try to detect whether lseek64() and off64_t are available. */
        int fd = vm_file(vm_registers[0]);
        off_t offset = (off_t)vm_registers[2] | (((off_t)vm_registers[3] << 16) << 16);
        offset = lseek(fd, offset, base);
        vm_registers[0] = (offset == -1) ? VM_ERR_IO : 0;
    }

    #elif defined _WIN32
        #error TODO _WIN32 fseek

    #else
    {
        /*
         * There are platform-specific extensions for 64-bit seek. POSIX 2001 has
         * fseeko() and ftello() for example. These are not in C89, and they
         * require `long long` which is not in C89 either. If we have POSIX
         * we'll be using lseek() (or lseek64()) anyway so there's no point in
         * trying to use them.
         */
        FILE* file = vm_file(vm_registers[0]);
        long offset = (long)vm_registers[2] | (((long)vm_registers[3] << 16) << 16);
        int ret = fseek(file, offset, base);
        vm_registers[0] = ret ? VM_ERR_IO : 0;
    }
    #endif
}

static void vm_ftell(void) {

    #ifdef VM_POSIX
    {
        uint32_t addr = vm_registers[1];
        int fd = vm_file(vm_registers[0]);
        off_t pos = lseek(fd, 0, SEEK_CUR);
        if (pos == -1) {
            vm_registers[0] = VM_ERR_IO;
            return;
        }

        vm_store_u32(addr, (uint32_t)pos);
        vm_store_u32(addr + 4,
                (sizeof(off_t) >= 8) ?
                    (uint32_t)(pos >> 32) :
                    0);
        vm_registers[0] = 0;
    }


    #elif defined _WIN32
        #error TODO _WIN32 fseek

    #else
    {
        FILE* file = vm_file(vm_registers[0]);
        long pos = ftell(file);
        if (pos == -1) {
            vm_registers[0] = VM_ERR_IO;
            return;
        }

        {
            /* As with fseek() we shift twice in case `long` or `off_t` is only
             * 32 bits. We convert to unsigned first to get an unsigned shift. */
            unsigned long upos = (unsigned long)pos;
            uint32_t addr = vm_registers[1];
            vm_store_u32(addr, (uint32_t)upos);
            vm_store_u32(addr + 4, (uint32_t)((upos >> 16) >> 16));
            vm_registers[0] = 0;
        }
    }
    #endif
}

static void vm_ftrunc(void) {
    uint32_t size_low = vm_registers[1];
    uint32_t size_high = vm_registers[2];

    /*
     * There is no standard C way to truncate an open file. For now we use
     * platform-specific functions.
     */

    #ifdef VM_POSIX
        /* On POSIX systems we call ftruncate(). */
        int fd = vm_file(vm_registers[0]);
        off_t upos = (off_t)size_low | (((off_t)size_high << 16) << 16);
        int ret = ftruncate(fd, upos);

    #elif defined _WIN32
        /* On Windows we have _chsize(). There is also _chsize_s() which is
         * 64-bit but our fseek()/ftell() functions aren't currently using
         * corresponding 64-bit functions so right now there's no point. */
        FILE* file = vm_file(vm_registers[0]);
        unsigned long upos = (unsigned long)size_low |
                (((unsigned long)size_high << 16) << 16);
        int ret;
        fflush(file);
        ret = _chsize(fileno(file), upos);

    #else
        /* TODO make this work with only standard C. If size is zero, freopen()
         * the file in "wb" mode; otherwise rename the file to a temporary and
         * copy the desired bytes. The Onramp bootstrap process only ever calls
         * this with size zero. */
        #error "ftrunc is not implemented on this platform."
    #endif

    vm_registers[0] = ret ? VM_ERR_GENERIC : 0;
}

static void vm_chmod(void) {

    /* There is nothing like chmod() in standard C. It's only relevant for
     * better integration into UNIX systems. */

    #ifdef VM_POSIX
        uint32_t path_addr = vm_registers[0];
        uint32_t mode = vm_registers[1];
        const char* path;
        vm_check_string(path_addr);
        path = (const char*)vm_memory + path_addr;
        vm_registers[0] = chmod(path, mode) ? VM_ERR_GENERIC : 0;

    #else
        vm_registers[0] = 0;
    #endif
}

static void vm_sys(uint8_t syscall) {
    /*printf("%u %u\n",arg1,arg2);*/

    switch (syscall) {
        case 0x00: /* halt */
            vm_halt();
            return;
        case 0x01: /* time */
            vm_time();
            return;
        case 0x03: /* fopen */
            vm_fopen();
            return;
        case 0x04: /* fclose */
            vm_fclose();
            return;
        case 0x05: /* fread */
            vm_fread();
            return;
        case 0x06: /* fwrite */
            vm_fwrite();
            return;
        case 0x07: /* fseek */
            vm_fseek();
            return;
        case 0x08: /* ftell */
            vm_ftell();
            return;
        case 0x09: /* ftrunc */
            vm_ftrunc();
            return;
        case 0x11: /* chmod */
            vm_chmod();
            return;
        default:
            break;
    }

    /* Unhandled syscall */
    /*vm_registers[0] = (uint32_t)(int32_t)(-1);*/
    vm_panic("Invalid syscall number");
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
    vm_check_aligned(rip);
    vm_check_valid(rip);
    opcode = vm_memory[rip];
    vm_check((opcode & 0xF0) == 0x70, "Invalid instruction");
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
        case 0x7F: /* sys */
            vm_check(arg2 == 0 && arg3 == 0, "Invalid instruction");
            vm_sys(arg1);
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
