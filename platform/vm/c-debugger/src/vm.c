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
 * This is a rich implementation of the Onramp VM in C with a built-in
 * debugger.
 *
 * Pass -d as the first argument to start a program in the debugger.
 */

#include "vmcommon.h"
#include "debug.h"
#include "terminal.h"

#include <time.h>
#include <inttypes.h>
#include <poll.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#ifdef _WIN32
    #include <direct.h>
    extern char** _environ;
    #define ghost_environ _environ
    #define mkdir(path, mode) ((void)(mode), _mkdir(path))
#else
    extern char** environ;
    #define ghost_environ environ
#endif

#if !vm_ghost_has(vm_ghost_stdint_h)
    #error <stdint.h> is required.
#endif

struct vm_t;
static void print_callstack(void);
static void vm_print_stats(struct vm_t* vm);

// Uncomment to print stats such as memory usage on exit. This is disabled by
// default to avoid impact on performance.
//#define VM_ENABLE_STATS

/* TODO should never panic, any error should break into the debugger */
/* TODO actually no, we should only break into the debugger if we were running
 * under it. This should be usable as a normal VM unless set to debug. */
// TODO maybe a separate command-line option, don't run in debugger by default
// but break into it if something goes wrong
vm_ghost_noreturn
static void panic(const char* e) {
    fprintf(stderr, "VM PANIC: %s\n", e);
    print_callstack();
    //vm_ghost_debugbreak();
    exit(125);
}

/* array indices of named registers */
#define VM_RSP 0xC  /* stack pointer */
#define VM_RFP 0xD  /* frame pointer */
#define VM_RPP 0xE  /* program pointer */
#define VM_RIP 0xF  /* instruction pointer */

/* opcodes */
#define VM_ADD  0x70  /* add */
#define VM_SUB  0x71  /* subtract */
#define VM_MUL  0x72  /* multiply */
#define VM_DIVU 0x73  /* divide unsigned */
#define VM_AND  0x74  /* bitwise and */
#define VM_OR   0x75  /* bitwise or */
#define VM_SHL  0x76  /* shift left */
#define VM_SHRU 0x77  /* shift right unsigned */
#define VM_LDW  0x78  /* load word */
#define VM_STW  0x79  /* store word */
#define VM_LDB  0x7A  /* load byte */
#define VM_STB  0x7B  /* store byte */
#define VM_IMS  0x7C  /* immediate short */
#define VM_LTU  0x7D  /* less than unsigned */
#define VM_JZ   0x7E  /* jump if zero */

/* syscalls */
#define VM_EXIT      0
#define VM_PANIC     1
#define VM_TIME      2
#define VM_OPEN      3
#define VM_CLOSE     4
#define VM_READ      5
#define VM_WRITE     6
#define VM_SEEK      7
#define VM_TELL      8  // replaced by size
#define VM_SIZE      8
#define VM_TRUNC     9
#define VM_DOPEN     10
#define VM_DCLOSE    11
#define VM_DIRENT    12
#define VM_STAT      13
#define VM_RENAME    14
#define VM_SYMLINK   15
#define VM_DELETE    16
#define VM_CHMOD     17
#define VM_MKDIR     18
#define VM_RMDIR     19
#define VM_SPAWN     20
#define VM_WAITPID   21
#define VM_DEBUG     22
#define VM_ALLOC     23
#define VM_FREE      24
#define VM_FORBIDDEN_SYSCALL 0xEEEEEEEE
#define VM_SYSCALL_COUNT 25u

/* process info table */
#define VM_PIT_MAJOR_VERSION 0
#define VM_HEAP_START 4
#define VM_SYSCALL_TABLE 8
#define VM_INPUT 12
#define VM_OUTPUT 16
#define VM_ERROR 20
#define VM_ARGS 24
#define VM_ENVIRON 28
#define VM_WORKDIR 32
#define VM_CAPABILITIES 36
#define VM_PIT_SYSCALL_COUNT_FIELD 40
#define VM_PIT_MINOR_VERSION 40
#define VM_PIT_COUNT_FIELD 44
#define VM_PIT_FREE_REGIONS 44
#define VM_PIT_COUNT 12
#define VM_PIT_SIZE (4*VM_PIT_COUNT)

// errors
#define VM_ERROR_GENERIC        0xFFFFFFFF
#define VM_ERROR_NO_SUCH_PATH   0xFFFFFFFE
#define VM_ERROR_IO             0xFFFFFFFD
#define VM_ERROR_UNSUPPORTED    0xFFFFFFFC
#define VM_ERROR_TRY_LATER      0xFFFFFFFB
#define VM_ERROR_END_OF_FILE    0xFFFFFFFA
#define VM_ERROR_OVERFLOW       0xFFFFFFF9

// file types
#define VM_STAT_UNKNOWN 0
#define VM_STAT_REGULAR 1
#define VM_STAT_DIRECTORY 2
#define VM_STAT_DEVICE 3
#define VM_STAT_STREAM 4

/* register and memory value on start */
#define VM_DEFAULT_MEMORY 0xDEADDEAD

/* address to be assigned to rip to make a syscall */
#define VM_SYSCALL_ADDRESS 0xAAAAAAAA

/* Files. We offset the file count in order to ensure programs are using them
 * correctly (and not just assuming 1 is stdout for example.) */
#define FILES_COUNT 16
#define FILES_OFFSET (INT_MAX-FILES_COUNT-1)

// Uncomment this to get warnings about unclosed file handles. This isn't on by
// default because it's not an error to leave files unclosed.
//#define WARN_UNCLOSED_FILES



static bool strace_enabled = false;

static void strace(const char* format, ...) {
    if (!strace_enabled)
        return;
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}



/*
 * Breakpoints
 *
 * TODO none of this is implemented yet
 */

typedef enum breakpoint_type_t {
    breakpoint_type_read,    // value at address is read
    breakpoint_type_write,   // value at address is written to (whether or not it's different)
    breakpoint_type_change,  // value at address changes (i.e. written value is different)
    breakpoint_type_match,   // value at address matches a particular value
    breakpoint_type_execute, // instruction pointer reaches address
} breakpoint_type_t;

typedef struct breakpoint_t {
    breakpoint_type_t type;
    uint32_t value; // value for change or match
    int size;       // size of value for change or match, 8 or 32
    int address;

    // source information
    #if 0
    string_t filename;
    string_t function;
    string_t variable;
    #endif
    int line;
    int offset;
    bool relative; // whether address is rpp-relative or absolute
} breakpoint_t;

#if 0
static breakpoint_t* breakpoint_new(const char* definition) {
    // TODO parse definition. if invalid, print breakpoint location syntax and
    // return null. if return is null, prompt will prompt again, command line
    // will exit.

    // need syntax for:
    // values: read/write/change/match at address (rpp-relative or absolute) and at variable[+offset]
    // functions: file+line[+offset], function[+offset]
    return vm_ghost_null;
}

static void breakpoint_format(const breakpoint_t* breakpoint, vm_ghost_string_t* out) {
    // TODO turn breakpoint source information back into formatted string
}
#endif



/*
 * VM
 */

typedef struct filedata_t {
    FILE* file;
    DIR* dir;

    // True if we just generated ERROR_TRY_LATER. When error generation is on,
    // we always return ERROR_TRY_LATER from read and write calls unless the
    // last error returned was ERROR_TRY_LATER. This tests retry behaviour on
    // all reads and writes.
    bool generated_error_later;
} filedata_t;

static inline bool filedata_is_open(filedata_t* filedata) {
    return filedata->file || filedata->dir;
}

typedef enum step_t {
    step_in,   // run only the current instruction
    step_over, // run until the next instruction
    step_out,  // run until the current function returns
    step_exit, // run until the current program exits
    step_run,  // run indefinitely
} step_t;

typedef struct vm_t {
    const char* filename;
    //char* root_path;
    uint32_t registers[16];

    uint8_t* memory;
    uint32_t memory_base; /* base address in virtual space */
    uint32_t memory_size;

    #ifdef VM_ENABLE_STATS
    uint8_t* memory_mask; // bit mask of 4KB pages touched
    size_t memory_mask_size;
    #endif

    int version;
    bool generate_error_later;

    filedata_t files[FILES_COUNT];
    uint32_t recent_addrs[3];
    bool running;
    bool debugger_active;
} vm_t;

// TODO fix this
vm_t* static_vm;

static inline bool vm_is_addr_aligned(vm_t* vm, uint32_t addr) {
    (void)vm;
    return (addr & 3) == 0;
}

static inline bool vm_is_addr_valid(vm_t* vm, uint32_t addr) {
    if (addr < vm->memory_base)
        return false;
    if (addr >= vm->memory_base + vm->memory_size)
        return false;
    return true;
}

static bool vm_is_buffer_valid(vm_t* vm, uint32_t addr, uint32_t size) {
    if (size == 0) {
        return true;
    }
    return (addr + size > addr) && \
        vm_is_addr_valid(vm, addr) && \
        vm_is_addr_valid(vm, addr + size - 1);
}

static bool vm_is_string_valid(vm_t* vm, uint32_t addr) {
    while (true) {
        if (!vm_is_addr_valid(vm, addr))
            return false;
        if (vm->memory[addr - vm->memory_base] == 0)
            break;
        ++addr;
    }
    return true;
}

vm_ghost_always_inline
static void vm_mark_page(vm_t* vm, uint32_t addr) {
    #ifdef VM_ENABLE_STATS
    size_t page = (addr - vm->memory_base) >> 12;
    vm->memory_mask[page >> 3] |= (1 << (page & 7));
    #endif
}

static void vm_store_u32(vm_t* vm, uint32_t addr, uint32_t value) {
    //printf("storeu32 addr 0x%X rpp 0x%X value 0x%X\n",addr,vm->registers[VM_RPP],value);
    if (!vm_is_addr_valid(vm, addr))
        panic("Invalid address");
    if (!vm_is_addr_aligned(vm, addr))
        panic("Address not aligned");
    //printf("%u %u\n",addr,value);
    vm_ghost_store_le_u32(vm->memory + (addr - vm->memory_base), value);
    vm_mark_page(vm, addr);
}

static uint32_t vm_load_u32(vm_t* vm, uint32_t addr) {
    //printf("loadu32 addr 0x%X rpp 0x%X ",addr,vm->registers[VM_RPP]);
    if (!vm_is_addr_valid(vm, addr)){
        panic("Invalid address");
    }
    if (!vm_is_addr_aligned(vm, addr))
        panic("Address not aligned");
    uint32_t ret = vm_ghost_load_le_u32(vm->memory + (addr - vm->memory_base));
    //printf("value 0x%X\n",ret);
    return ret;
}

static void vm_store_u8(vm_t* vm, uint32_t addr, uint8_t value) {
    if (!vm_is_addr_valid(vm, addr))
        panic("Invalid address");
    vm->memory[addr - vm->memory_base] = value;
    vm_mark_page(vm, addr);
}

static uint8_t vm_load_u8(vm_t* vm, uint32_t addr) {
    if (!vm_is_addr_valid(vm, addr))
        panic("Invalid address");
    return vm->memory[addr - vm->memory_base];
}

static void usage(const char* command) {
    fputs("\n", stderr);
    fprintf(stderr, "Usage: %s [vm options] <program> [program options]\n", command);
    fputs("\n", stderr);

    fputs("VM options:\n", stderr);
    fputs("    -d                start the program paused in the debugger\n", stderr);
    //fputs("    -b <location>     add a breakpoint at the given location\n", stderr);
    //fputs("    -m <size>         size of program-accessible address space\n", stderr);
    //fputs("    -r <path>         path to root of filesystem\n", stderr);
    fputs("    -s                trace system calls\n", stderr);
    fputs("    -V <version>      version of Onramp spec to emulate (2, 3, 4); default 3\n", stderr);
    fputs("    --                end of VM arguments (use to run programs that start with '-')\n", stderr);
    fputs("    -h, --help        print this help\n", stderr);
    fputs("    --later           generate spurious ERROR_TRY_LATER wherever possible\n", stderr);
    fputs("\n", stderr);

    //fputs("Breakpoint location syntax:\n", stderr);
    //fputs("    TODO\n", stderr);
    //fputs("\n", stderr);
}

static size_t vm_store_string(vm_t* vm, size_t addr, const char* str) {
    size_t size = strlen(str) + 1;
    if (!vm_is_buffer_valid(vm, addr, size)) {
        fputs("ERROR: Out of memory storing string.\n", stderr);
        exit(125);
    }
    memcpy(vm->memory + (addr - vm->memory_base), str, size);
    return addr + size;
}

/**
 * Copies the given null-terminated string array into virtual memory starting
 * at the given address. Returns the aligned address after the end of the
 * array.
 *
 * This is used to load command-line options and environment variables into the
 * program.
 */
static size_t vm_store_string_array(vm_t* vm, size_t addr, const char** strings) {

    // count strings
    size_t count = 0;
    for (; strings[count] != NULL; ++count) {}

    // setup array
    size_t array = addr;
    addr += (count + 1) * 4;
    if (!vm_is_buffer_valid(vm, array, addr - array)) {
        fputs("ERROR: Out of memory storing string.\n", stderr);
        exit(125);
    }

    // load strings
    for (; *strings; ++strings) {
        vm_store_u32(vm, array, addr);
        array += 4;
        addr = vm_store_string(vm, addr, *strings);
    }
    vm_store_u32(vm, array, 0);

    // align address
    addr = (addr + 0x3u) & ~0x3u;
    return addr;

}

static size_t vm_parse_args(vm_t* vm, int argc, const char* argv[], uint32_t pit_addr, uint32_t addr) {
    int i;

    // parse vm args
    // TODO use getopt or whatever
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            break;
        }

        // strace
        if (0 == strcmp(argv[i], "-s")) {
            strace_enabled = true;
            continue;
        }

        // debugger
        if (0 == strcmp(argv[i], "-d")) {
            // TODO we need to either turn on non-blocking or poll on input
            // when waiting for debugger commands
            fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
            vm->running = false;
            vm->debugger_active = true;
            continue;
        }

        // version
        if (0 == strncmp(argv[i], "-V", 2)) {
            const char* version_str;
            if (argv[i][2]) {
                version_str = argv[i] + 2;
            } else {
                ++i;
                if (!argv[i]) {
                    fputs("ERROR: Expected an argument after `-V`.\n", stderr);
                    usage(argv[0]);
                    exit(125);
                }
                version_str = argv[i];
            }
            if (1 != sscanf(version_str, "%i", &vm->version) ||
                    (vm->version < 2) || vm->version > 4)
            {
                fputs("ERROR: Version argument to `-V` invalid or out of range.\n", stderr);
                usage(argv[0]);
                exit(125);
            }
            continue;
        }

        // later
        if (0 == strcmp(argv[i], "--later")) {
            vm->generate_error_later = true;
            continue;
        }

        // usage
        if (0 == strcmp(argv[i], "--help") ||
                0 == strcmp(argv[i], "-h") ||
                0 == strcmp(argv[i], "-?"))
        {
            usage(argv[0]);
            exit(0);
        }

        // explicit end of argument list
        if (0 == strcmp(argv[i], "--")) {
            break;
        }

        fprintf(stderr, "ERROR: Unrecognized command-line argument: %s\n", argv[i]);
        usage(argv[0]);
        exit(125);
    }

    if (vm->generate_error_later && vm->version != 4) {
        fputs("ERROR: --later requires -V4.\n", stderr);
        exit(125);
    }

    // parse filename
    if (i == argc) {
        fputs("ERROR: No program filename specified.\n", stderr);
        usage(argv[0]);
        exit(125);
    }
    vm->filename = argv[i];

    // load strings
    vm_store_u32(vm, pit_addr + VM_ARGS, addr);
    addr = vm_store_string_array(vm, addr, argv + i);
    vm_store_u32(vm, pit_addr + VM_ENVIRON, addr);
    addr = vm_store_string_array(vm, addr, vm_ghost_const_cast(const char**, ghost_environ));

    // copy working dir
    vm_store_u32(vm, pit_addr + VM_WORKDIR, addr);
    size_t offset = addr - vm->memory_base;
    char* cwd = getcwd((char*)vm->memory + offset, vm->memory_size - offset);
    if (cwd == NULL) {
        fputs("ERROR: Failed to get working directory.\n", stderr);
        exit(125);
    }
    addr += strlen(cwd);

    // align address
    addr = (addr + 0x3u) & ~0x3u;

    return addr;
}

// Returns true if the given syscall was never used on version 2 and 3 VMs.
// These must never be called by programs on versions 2 and 3 (even if they
// were later implemented in version 4.)
static bool syscall_is_v3_forbidden(int num) {
    switch (num) {

        // removed:
        case VM_DOPEN:
        case VM_DCLOSE:
        case VM_SYMLINK:
        case VM_RMDIR:
        case VM_SPAWN:
        case VM_WAITPID:

        // never implemented or never used:
        case VM_STAT:
        case VM_ALLOC:
        case VM_FREE:
            return true;

        default:
            break;
    }
    return false;
}

static void vm_init(vm_t* vm, int argc, const char* argv[]) {
    memset(vm, 0, sizeof(*vm));

    // TODO
    (void)argc;
    (void)argv;

    /* TODO don't randomize automatically when engaging debugger (and allow command line option) */
    srand(time(NULL));
    vm->memory_base = 0x10000 ;//* (1 + (rand() & 0xFFF));

    /* TODO allow command-line option */
    vm->memory_size = 16 * 1024 * 1024;

    vm->running = true;
    vm->version = 4;
            vm->generate_error_later = true;

    // allocate and dead-fill memory
    vm->memory = vm_ghost_alloc_array(uint8_t, vm->memory_size);
    for (size_t i = 0; i + 3 < vm->memory_size; i += 4) {
        vm_ghost_store_le_u32(vm->memory + i, VM_DEFAULT_MEMORY);
    }

    #ifdef VM_ENABLE_STATS
    // allocate memory usage mask
    size_t mask_bits = (vm->memory_size + 4095) >> 12;
    vm->memory_mask_size = (mask_bits + 7) >> 3;
    vm->memory_mask = calloc(vm->memory_mask_size, sizeof(uint8_t));
    #endif

    uint32_t addr = vm->memory_base;

    // Allocate a free memory region before the PIT. (This will only be usable
    // on version 4 or later, but we can't tell the version before parsing
    // args, which requires the PIT.)
    uint32_t region_1_start = addr;
    addr += 0x10000;
    uint32_t region_1_end = addr;

    // allocate the PIT
    uint32_t pit = addr;
    addr += VM_PIT_SIZE;

    /* parse arguments and environment variables */
    addr = vm_parse_args(vm, argc, argv, pit, addr);

    // Setup syscall table
    // All syscalls share the same function address. The context is the syscall
    // number.
    uint32_t syscall_table = addr;
    vm_store_u32(vm, pit + VM_SYSCALL_TABLE, syscall_table);
    addr += VM_SYSCALL_COUNT * 8u;
    for (uint32_t i = 0; i < VM_SYSCALL_COUNT; ++i) {
        if (vm->version < 4 && syscall_is_v3_forbidden(i)) {
            vm_store_u32(vm, syscall_table + i * 8, VM_SYSCALL_ADDRESS);
            vm_store_u32(vm, syscall_table + i * 8 + 4, VM_FORBIDDEN_SYSCALL);
        } else if (i == VM_STAT || i == VM_RENAME || i == VM_ALLOC || i == VM_FREE) {
            // TODO these are not implemented yet
            vm_store_u32(vm, syscall_table + i * 8, 0);
            vm_store_u32(vm, syscall_table + i * 8 + 4, 0);
        } else {
            vm_store_u32(vm, syscall_table + i * 8, VM_SYSCALL_ADDRESS);
            vm_store_u32(vm, syscall_table + i * 8 + 4, i);
        }
    }

    /* setup files */
    vm->files[0].file = stdin;
    vm->files[1].file = stdout;
    vm->files[2].file = stderr;
    vm_store_u32(vm, pit + VM_INPUT, FILES_OFFSET);
    vm_store_u32(vm, pit + VM_OUTPUT, FILES_OFFSET + 1);
    vm_store_u32(vm, pit + VM_ERROR, FILES_OFFSET + 2);

    // On version 4 VMs, allocate space for five words, enough for a
    // null-terminated list of/ two memory regions.
    uint32_t free_memory_regions = addr;
    addr += 5 * 4;

    // Round up the program address to a multiple of 0x10000 to make it easier
    // to debug. This becomes the second additional free memory region in
    // version 4.
    uint32_t region_2_start = addr;
    addr += 4; // make sure the region isn't empty
    addr = (addr + 0x10000 - 1) & ~(0x10000 - 1);
    uint32_t region_2_end = addr;

    // Write the free memory regions
    if (vm->version >= 4) {
        vm_store_u32(vm, pit + VM_PIT_FREE_REGIONS, free_memory_regions);
        vm_store_u32(vm, free_memory_regions,      region_1_start);
        vm_store_u32(vm, free_memory_regions + 4,  region_1_end - region_1_start);
        vm_store_u32(vm, free_memory_regions + 8,  region_2_start);
        vm_store_u32(vm, free_memory_regions + 12, region_2_end - region_2_start);
        vm_store_u32(vm, free_memory_regions + 16, 0);
    }

    /* load program */
    FILE* file = fopen(vm->filename, "rb");
    if (file == vm_ghost_null) {
        panic("Couldn't open program");
    }
    uint32_t start = addr;
    uint32_t end = vm->memory_base + vm->memory_size;
    for (;;) {
        uint32_t p = addr - vm->memory_base;
        size_t ret = fread(vm->memory + p, 1, vm->memory_size - p, file);
        if (ret == 0) {
            if (feof(file))
                break;
            panic("Error reading program!");
        }
        addr += (uint32_t)ret;
    }
    if (addr == start) {
        panic("Program is empty.");
    }
    fclose(file);

    // if the program starts with "#!" or "REM", skip the first 128 bytes
    if ((vm_load_u8(vm, start) == '#' && vm_load_u8(vm, start + 1) == '!') ||
            (vm_load_u8(vm, start) == 'R' &&
             vm_load_u8(vm, start + 1) == 'E' &&
             vm_load_u8(vm, start + 2) == 'M'))
    {
        start += 128;
    }

    // TODO we should be able to load the debug info lazily, e.g. only when we
    // need to print a stack trace
    //printf("LOADING DEBUG INFO\n");
    debug_load(vm->filename, start);

    /* check program preamble */
    if (vm_load_u32(vm, start) != 0x726E4F7Eu ||
            vm_load_u32(vm, start + 4) != 0x706D617Eu ||
            vm_load_u32(vm, start + 8) != 0x2020207Eu)
    {
        fprintf(stderr,
                "WARNING: Program %s does not start with `~Onr~amp~   ` preamble\n",
                vm->filename);
    }

    // round up the heap address to a multiple of 0x10000 again
    addr = (addr + 0x10000 - 1) & ~(0x10000 - 1);

    // set capabilities
    int interactive = (vm->version >= 4 && isatty(0) && isatty(1)) << 3;
    int capabilities = interactive |
            (raw_input_enabled
                ? 0 // no echo, non-canonical
                : 5 // echo, canonical
            );
    vm_store_u32(vm, pit + VM_CAPABILITIES, capabilities);

    // set up the rest of the process info table
    vm_store_u32(vm, pit + VM_PIT_MAJOR_VERSION, vm->version);
    if (vm->version >= 4) {
        vm_store_u32(vm, pit + VM_PIT_MINOR_VERSION, 0);
    }
    vm_store_u32(vm, pit + VM_HEAP_START, addr);
    if (vm->version == 3) {
        vm_store_u32(vm, pit + VM_PIT_SYSCALL_COUNT_FIELD, VM_SYSCALL_COUNT);
        vm_store_u32(vm, pit + VM_PIT_COUNT_FIELD, VM_PIT_COUNT);
    }

    // set initial register values
    vm->registers[0] = pit;
    for (size_t i = 1; i <= VM_RFP; ++i)
        vm->registers[i] = VM_DEFAULT_MEMORY;
    vm->registers[VM_RSP] = end;
    vm->registers[VM_RPP] = start;
    vm->registers[VM_RIP] = start;

    // initialize the debugger memory views
    vm->recent_addrs[0] = start;
    vm->recent_addrs[1] = addr;
    vm->recent_addrs[2] = end;
}

static void vm_destroy(vm_t* vm) {
    free(vm->memory);
    //free(vm->root_path);
}

static uint32_t vm_parse_mix(vm_t* vm, uint8_t b) {
    if (b <= 0x7Fu)
        return b;
    if (b >= 0x90u)
        return (uint32_t)b | 0xFFFFFF00u;
    if ((b & 0xF0u) == 0x80u)
        return vm->registers[b & 0x0Fu];
    panic("Invalid value for mix-type argument");
}

vm_ghost_always_inline
static uint8_t vm_parse_register(vm_t* vm, uint8_t b) {
    (void)vm;
    if ((b & 0xF0u) != 0x80u)
        panic("Invalid register");
    return b & 0x0Fu;
}

vm_ghost_noinline
static filedata_t* vm_filedata(vm_t* vm, uint32_t handle) {
    handle -= FILES_OFFSET;
    if (handle >= (uint32_t)vm_ghost_array_count(vm->files))
        panic("File handle is invalid");
    return &vm->files[handle];
}

static uint32_t vm_exit(vm_t* vm) {
    // TODO pause debugger
    strace("sys exit() %i\n", vm->registers[0]);
    vm_print_stats(vm);

    #ifdef WARN_UNCLOSED_FILES
    // The first three are the standard streams (stdin, stdout and stderr.) We
    // don't warn about leaving them unclosed.
    // TODO later we will allow closing the standard streams in which case
    // their file handles could be reused. We should actually check whether
    // these are the standard streams.
    for (unsigned i = 3; i < vm_ghost_array_count(vm->files); ++i) {
        if (vm->files[i]) {
            fprintf(stderr, "WARNING: Unclosed handle: %i 0x%x\n", i, i + FILES_OFFSET);
        }
    }
    #endif

    exit(vm_parse_mix(vm, vm->registers[0]));
    return VM_ERROR_GENERIC;
}

static uint32_t vm_time(vm_t* vm) {
    struct timespec time;
    if (0 != clock_gettime(CLOCK_REALTIME, &time)) {
        return VM_ERROR_UNSUPPORTED;
    }
    strace("sys time() %" PRIi64 " s %u ns", (uint64_t)time.tv_sec, (unsigned)time.tv_nsec);

    uint32_t addr = vm->registers[0];
    vm_store_u32(vm, addr, (uint32_t)time.tv_sec);
    vm_store_u32(vm, addr + 4, (uint32_t)((uint64_t)time.tv_sec >> 32));
    vm_store_u32(vm, addr + 8, time.tv_nsec);
    return 0;
}

static uint32_t vm_debug(vm_t* vm) {
    strace("sys debug()");

    uint32_t program_addr = vm->registers[0];
    uint32_t path_addr = vm->registers[1];

    strace(" addr 0x%x", program_addr);
    if (!vm_is_addr_valid(vm, program_addr)) {
        strace(" not mapped!");
        return VM_ERROR_GENERIC;
    }

    if (path_addr == 0) {
        strace(" unload");
        debug_unload(program_addr);
        return 0;
    }

    if (!vm_is_string_valid(vm, path_addr)) {
        strace(" load path not mapped!", program_addr, path_addr);
        // TODO report error in some way
        return VM_ERROR_GENERIC;
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    strace(" load path \"%s\"", full_path);
    debug_load(full_path, program_addr);

    return 0;
}

static uint32_t vm_open(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    uint32_t mode = vm->registers[1];
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    strace("sys open() path \"%s\" mode %i", path, mode);

    // find a free handle
    uint32_t file_index = UINT32_MAX;
    for (size_t i = 0; i < vm_ghost_array_count(vm->files); ++i) {
        if (!filedata_is_open(&vm->files[i])) {
            file_index = i;
            break;
        }
    }
    if (file_index == UINT32_MAX) {
        // too many open files
        return VM_ERROR_OVERFLOW;
    }
    filedata_t* filedata = &vm->files[file_index];

    // open it in the correct mode
    if (mode) {
        // Try to open the existing file read/write. If it fails, it's probably
        // because it doesn't exist, so try opening for writing to create it.
        // (We're not concerned with race conditions in Onramp.)
        filedata->file = fopen(path, "r+b");
        if (filedata->file == NULL) {
            filedata->file = fopen(path, "w+b");
        }
    } else {
        // Try to open it as a directory first.
        filedata->dir = opendir(path);
        if (filedata->dir == NULL) {
            filedata->file = fopen(path, "rb");
        }
    }

    if (!filedata_is_open(filedata)) {
        switch (errno) {
            case ENOENT:
                return VM_ERROR_NO_SUCH_PATH;
            case EACCES:
            case EISDIR:
                return VM_ERROR_UNSUPPORTED;
            default:
                return VM_ERROR_GENERIC;
        }
    }

    filedata->generated_error_later = false;
    return file_index + FILES_OFFSET;
}

static uint32_t vm_close(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    strace("sys close() handle 0x%x", handle);
    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }

    if (filedata->dir) {
        closedir(filedata->dir);
        filedata->dir = NULL;
        return 0;
    }

    // The program is allowed to close standard streams but we always ignore
    // closing stderr so we can print errors and we ignore closing the other
    // streams if we're in debugger mode.
    FILE* file = filedata->file;
    if (!(file == stderr ||
            (vm->debugger_active && (file == stdin || file == stdout))))
    {
        fclose(file);
    }
    filedata->file = NULL;
    return 0;
}

static uint32_t vm_read(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint32_t addr = vm->registers[1];
    uint32_t count = vm->registers[2];
    strace("sys read() handle 0x%x addr 0x%x count %u", handle, addr, count);
    filedata_t* filedata = vm_filedata(vm, handle);

    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    if (count == 0) {
        panic("A read of zero bytes is not allowed.");
    }
    if (!vm_is_buffer_valid(vm, addr, count)) {
        panic("Invalid buffer given to syscall read.");
    }

    // generate ERROR_TRY_LATER if the user has requested it
    if (vm->generate_error_later && !filedata->generated_error_later) {
        filedata->generated_error_later = true;
        return VM_ERROR_TRY_LATER;
    }
    filedata->generated_error_later = false;

    FILE* file = filedata->file;
    if (file == stdin) {
        fflush(stderr);
        fflush(stdout);
    }

    uint8_t* buffer = vm->memory + (addr - vm->memory_base);

    // In order to test that stages correctly handle short reads, we limit the
    // number of bytes that can be read at once.
    #ifndef MAX_READ_SIZE
    #define MAX_READ_SIZE 127
    #endif
    if (count > MAX_READ_SIZE) {
        count = MAX_READ_SIZE;
    }

    if (file == stdin) {
        // We want our input to be non-blocking. We use poll() with zero
        // timeout to check if input is available.
        // TODO we should do this with all files so e.g. the program can open
        // UDS streams non-blocking
        struct pollfd fds = {STDIN_FILENO, POLLIN, 0};
        int ret;
        do {
            ret = poll(&fds, 1, 0);
        } while (ret == EINTR);
        if (ret < 0) {
            return VM_ERROR_GENERIC;
        }
        if (fds.revents == POLLERR) {
            return VM_ERROR_IO;
        }
        if (fds.revents != POLLIN) {
            if (fds.revents == POLLHUP) {
                // TODO we need to return a different error code for closed input
            }
            return 0;
        }
    }

    size_t ret = fread(buffer, 1, count, file);
    if (ret == 0) {
        if (feof(file)) {
            if (vm->version < 4) {
                // Old VM versions reported EOF by returning zero.
                return 0;
            }
            return VM_ERROR_END_OF_FILE;
        }
        return VM_ERROR_IO;
    }
    return (uint32_t)ret;
}

static uint32_t vm_write(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint32_t addr = vm->registers[1];
    uint32_t count = vm->registers[2];
    strace("sys write() handle 0x%x addr 0x%x count %u", handle, addr, count);
    filedata_t* filedata = vm_filedata(vm, handle);

    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    if (count == 0) {
        panic("A write of zero bytes is not allowed.");
    }
    if (!vm_is_buffer_valid(vm, addr, count)) {
        panic("Invalid buffer given to syscall write.");
    }

    // generate ERROR_TRY_LATER if the user has requested it
    if (vm->generate_error_later && !filedata->generated_error_later) {
        filedata->generated_error_later = true;
        return VM_ERROR_TRY_LATER;
    }
    filedata->generated_error_later = false;

    FILE* file = filedata->file;
    if (file == stdin) {
        fflush(stderr);
        fflush(stdout);
    }

    uint8_t* buffer = vm->memory + (addr - vm->memory_base);

    // In order to test that stages correctly handle short writes, we limit the
    // number of bytes that can be written at once. The default for writes is
    // smaller than reads to test correct behaviour of read->write loops.
    #ifndef MAX_WRITE_SIZE
    #define MAX_WRITE_SIZE 59
    #endif
    if (count > MAX_WRITE_SIZE) {
        count = MAX_WRITE_SIZE;
    }

    size_t ret;
    if (file == stdout || file == stderr) {
        // Setting stdin to non-blocking apparently causes stdout to fail on
        // fwrite() calls.  TODO the debugger pretty much requires POSIX so we
        // should get rid of the C file API entirely and just use POSIX.
        // TODO finally figured out why this is, setting stdin to nonblock sets
        // it for stdout as well.
        ssize_t sret = write(fileno(file), buffer, count);
        if (sret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                if (vm->version < 4) {
                    // Old VM versions reported EWOULDBLOCK by returning zero.
                    return 0;
                }
                return VM_ERROR_TRY_LATER;
            }
            // TODO error codes
            return VM_ERROR_GENERIC;
        }
        ret = (size_t)sret;
    } else {
        ret = fwrite(buffer, 1, count, file);
        if (ret == 0) {
            panic("Error writing file!");
        }
    }
    /*
    if (ret != count) {
        printf("\n\n ret %zi count %u\n\n",ret,count);
        panic("Error writing file!");
    }
    */

    // We need to flush to ensure this doesn't interfere with our debugger.
    // TODO only flush when debugger NOT running
    // TODO why not running? shouldn't we only flush when running?
    /*if (file == stdout || file == stderr) {
        fflush(file);
    }*/

    return (uint32_t)ret;
}

static uint32_t vm_seek(vm_t* vm) {
    uint32_t handle = vm->registers[0];

    // seek parameters changed between versions 3 and 4.
    uint32_t base, offset_low, offset_high;
    if (vm->version < 4) {
        base = vm->registers[1];
        offset_low = vm->registers[2];
        offset_high = vm->registers[3];
    } else {
        base = 0;
        offset_low = vm->registers[1];
        offset_high = vm->registers[2];
    }

    int64_t offset = (int64_t)((uint64_t)offset_low | ((uint64_t)offset_high << 32));

    if (vm->version < 4) {
        strace("sys seek() handle 0x%x base %u offset %" PRIi64, handle, base, offset);
    } else {
        strace("sys seek() handle 0x%x offset %" PRIi64, handle, offset);
    }

    if (base > 2) {
        panic("Invalid base given to syscall seek.");
    }

    // If compiled as 32 bits, we don't have large file support.
    // TODO use fseeko() where available or switch to POSIX APIs
    if (offset > LONG_MAX || offset < LONG_MIN) {
        strace(" overflow");
        return VM_ERROR_OVERFLOW;
    }

    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    filedata->generated_error_later = false;

    if (0 == fseek(filedata->file, offset,
            base == 0 ? SEEK_SET : base == 1 ? SEEK_CUR : SEEK_END))
        return 0;

    // TODO error codes
    return VM_ERROR_GENERIC;
}

static uint32_t vm_tell(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint32_t addr = vm->registers[1];
    strace("sys tell() handle 0x%x addr 0x%x", handle, addr);

    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    filedata->generated_error_later = false;

    long pos = ftell(filedata->file);
    if (pos < 0) {
        strace(" failed, must be stream");
        return VM_ERROR_UNSUPPORTED;
    }
    strace(" position %" PRIu64, (uint64_t)pos);

    vm_store_u32(vm, addr, (uint32_t)pos);
    vm_store_u32(vm, addr + 4, (uint32_t)(pos >> 32));
    return 0;
}

static uint32_t vm_trunc(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint64_t length = (uint64_t)vm->registers[1] | ((uint64_t)vm->registers[2] << 32);
    strace("sys trunc() handle 0x%x length %" PRIu64, vm->registers[0], (uint64_t)length);

    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    if (length != (uint64_t)(off_t)length) {
        return VM_ERROR_OVERFLOW;
    }
    filedata->generated_error_later = false;

    FILE* file = filedata->file;
    fflush(file);
    int ret = ftruncate(fileno(file), (off_t)length);
    if (ret == 0)
        return 0;
    strace(" failed, ret %i errno %i", ret, errno);
    // TODO error codes
    return VM_ERROR_GENERIC;
}

static uint32_t vm_size(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint32_t out_addr = vm->registers[1];
    strace("sys size() handle 0x%x addr 0x%x", handle, out_addr);

    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->file == NULL) {
        // It's a directory.
        strace(" failed, directory");
        return VM_ERROR_UNSUPPORTED;
    }
    filedata->generated_error_later = false;

    fflush(filedata->file);
    struct stat statbuf;
    int ret = fstat(fileno(filedata->file), &statbuf);
    if (ret != 0) {
        strace(" failed, must be stream");
        return VM_ERROR_UNSUPPORTED;
    }

    // The statbuf size is supposed to be signed but it should never be
    // negative. We check just in case so we don't get nonsense from our shifts
    // below.
    if (statbuf.st_size < 0) {
        strace(" size is negative??");
        return VM_ERROR_GENERIC;
    }
    strace(" size %" PRIu64, (uint64_t)statbuf.st_size);

    // Store size (shift twice in case off_t is 32 bits)
    vm_store_u32(vm, out_addr, (uint32_t)statbuf.st_size);
    vm_store_u32(vm, out_addr + 4, (uint32_t)((statbuf.st_size >> 16) >> 16));
    return 0;
}

static uint32_t vm_dirent(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    uint32_t buffer_addr = vm->registers[1];
    strace("sys dirent() handle 0x%x buffer 0x%x", handle, buffer_addr);
    if (!vm_is_buffer_valid(vm, buffer_addr, 1)) {
        panic("Invalid buffer passed to syscall dirent");
    }
    if (!vm_is_buffer_valid(vm, buffer_addr, 256)) {
        panic("Insufficient buffer passed to syscall dirent; must fit 256 bytes.");
    }

    filedata_t* filedata = vm_filedata(vm, handle);
    if (!filedata_is_open(filedata)) {
        panic("File handle is not open");
    }
    if (filedata->dir == NULL) {
        // It's not a directory.
        strace(" failed, not a directory");
        return VM_ERROR_UNSUPPORTED;
    }

    char* buffer = (char*)(vm->memory + (buffer_addr - vm->memory_base));
    for (;;) {
        errno = 0;
        struct dirent* dirent = readdir(filedata->dir);
        if (dirent == NULL) {
            if (errno) {
                // TODO parse out errno. We were able to open the directory so
                // if we can't read it we assume it's an I/O error.
                return VM_ERROR_IO;
            }

            // end of directory
            if (vm->version < 4) {
                // ERROR_END_OF_FILE didn't exist in v3 and earlier VMs. We
                // report the end of the directory with an empty string.
                *buffer = 0;
                memset(buffer + 1, 0xFF, 255);
                return 0;
            }
            // We fill the buffer with a non-zero value to ensure the program
            // doesn't try to use it (but we still put a null-terminator on the
            // end in case it's buggy.)
            memset(buffer, 0xFF, 255);
            buffer[255] = 0;
            return VM_ERROR_END_OF_FILE;
        }

        /* TODO enable this
        if (0 == strcmp(dirent->d_name, ".") || 0 == strcmp(dirent->d_name, "..")) {
            // test skipping these; the libc should simulate them
            continue;
        }
        */

        size_t len = strlen(dirent->d_name);
        if (len > 255) {
            // This isn't possible on Linux but some platforms may permit
            // longer file names. Onramp doesn't support such filenames so we
            // print a warning and skip them.
            fprintf(stderr, "VM warning: File name is too long: %s\n", dirent->d_name);
            continue;
        }

        memcpy(buffer, dirent->d_name, len + 1);

        // We clear the rest of the buffer with a non-zero value to ensure the
        // program doesn't rely on its previous values being preserved. The VM
        // is allowed to write to the entire buffer.
        memset(buffer + len + 1, 0xFF, 256 - len - 1);
        return 0;
    }
}

#if 0
// Converts stat.st_mode to Onramp
static int vm_stat_type(int mode) {
    if (S_ISREG(mode)) {
        return VM_STAT_REGULAR;
    }
    if (S_ISDIR(mode)) {
        return VM_STAT_DIRECTORY;
    }
    if (S_ISFIFO(mode)) {
        return VM_STAT_STREAM;
    }
    if (S_ISCHR(mode) || S_ISBLK(mode)) {
        return VM_STAT_DEVICE;
    }

    // symlinks (not in posix)
    #ifdef S_ISLNK
    if (S_ISLNK(mode)) {
        //return VM_STAT_SYMLINK;
        // TODO: symlinks are not supported yet. for now we aren't calling
        // lstat() so this shouldn't happen.
        panic("S_ISLNK?");
    }
    #endif

    // sockets (not in posix)
    #ifdef S_ISSOCK
    if (S_ISLNK(mode)) {
        return VM_STAT_DEVICE;
    }
    #endif

    // we don't know what it is!
    return VM_STAT_UNKNOWN;
}

static uint32_t vm_stat(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    strace("sys stat ");
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    strace("%s", full_path);

    uint32_t out_addr = vm->registers[1];
    if (!vm_is_buffer_valid(vm, out_addr, 8)) {
        fputs("ERROR: Invalid buffer.\n", stderr);
        exit(125);
    }

    struct stat statbuf;
    int ret = stat(full_path, &statbuf);
    if (ret != 0) {
        strace(" failed, ret %i errno %i", ret, errno);
        // TODO correct error codes
        return VM_ERROR_GENERIC;
    }

    // The statbuf size is supposed to be signed but it should never be
    // negative. We check just in case so we don't get nonsense from our shifts
    // below.
    if (statbuf.st_size < 0) {
        strace(" size is negative??");
        return VM_ERROR_GENERIC;
    }
    strace(" size %" PRIu64, (uint64_t)statbuf.st_size);

    // Store size (shift twice in case off_t is 32 bits)
    vm_store_u32(vm, out_addr, (uint32_t)statbuf.st_size);
    vm_store_u32(vm, out_addr + 4, (uint32_t)((statbuf.st_size >> 16) >> 16));

    return vm_stat_type(statbuf.st_mode);
}
#endif

static uint32_t vm_rename(vm_t* vm) {
    panic("TODO rename syscall not yet implemented");
}

static uint32_t vm_delete(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    strace("sys delete ");
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    strace("%s", full_path);
    int ret = remove(full_path);
    if (ret == 0) {
        return 0;
    }
    strace(" failed, ret %i errno %i", ret, errno);
    // TODO correct error codes
    return VM_ERROR_GENERIC;
}

static int vm_chmod(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));

    mode_t mode = vm->registers[1];
    if (mode != 0644 && mode != 0755) {
        panic("Invalid chmod mode");
    }
    chmod(full_path, mode);
    return 0;
}

static uint32_t vm_mkdir(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    strace("sys mkdir() path \"%s\"", full_path);

    if (0 == mkdir(full_path, 0755))
        return 0;
    return VM_ERROR_GENERIC;
}

vm_ghost_noinline
static void vm_syscall(vm_t* vm) {
    uint32_t syscall_number = vm->registers[9];
    int ret = 0;

    switch (syscall_number) {
        // misc
        case VM_PANIC: // panic forwarded to exit (TODO shouldn't forward, need to check that exit code is non-zero)
        case VM_EXIT:      ret = vm_exit(vm); break;
        case VM_TIME:      ret = vm_time(vm); break;
        // file
        case VM_OPEN:      ret = vm_open(vm); break;
        case VM_CLOSE:     ret = vm_close(vm); break;
        case VM_READ:      ret = vm_read(vm); break;
        case VM_WRITE:     ret = vm_write(vm); break;
        case VM_SEEK:      ret = vm_seek(vm); break;
        case VM_SIZE:      ret = (vm->version >= 4 ? vm_size(vm) : vm_tell(vm)); break;
        case VM_TRUNC:     ret = vm_trunc(vm); break;
        // directory
        case VM_DIRENT:    ret = vm_dirent(vm); break;
        // filesystem
        //case VM_STAT:      ret = vm_stat(vm); break;
        case VM_RENAME:    ret = vm_rename(vm); break;
        case VM_DELETE:    ret = vm_delete(vm); break;
        case VM_CHMOD:     ret = vm_chmod(vm); break;
        case VM_MKDIR:     ret = vm_mkdir(vm); break;
        case VM_DEBUG:     ret = vm_debug(vm); break;
        case VM_FORBIDDEN_SYSCALL:
            // this syscall was never implemented or used in v2/v3. it must not
            // be called.
            panic("Forbidden syscall");
        default:
            panic("Unrecognized syscall");
    }

    strace(" == %i\n", ret); // TODO parse into error code
    vm->registers[0] = ret;

    // We quash all other caller-preserved registers to ensure that programs do
    // not depend on register preservation.
    for (size_t i = 1; i <= 0xB; ++i) {
        vm->registers[i] = 0xDEADDEAD;
    }

    // jump to return address
    uint32_t return_addr = vm_load_u32(vm, vm->registers[VM_RSP]);
    debug_callstack_pop(return_addr);
    vm->registers[VM_RIP] = return_addr;
}

static void vm_step(vm_t* vm) {
    /*
    if (!vm_is_addr_valid(vm, vm->registers[VM_RIP]))
        if (vm_syscall(vm))
            return;
            */

    uint32_t rip = vm->registers[VM_RIP];
    if (rip == VM_SYSCALL_ADDRESS) {
        vm_syscall(vm);
        return;
    }

    uint32_t instruction = vm_load_u32(vm, rip);
    vm->registers[VM_RIP] += 4;
    uint8_t opcode = (uint8_t)instruction;
    uint8_t arg1 = (uint8_t)(instruction >> 8);
    uint8_t arg2 = (uint8_t)(instruction >> 16);
    uint8_t arg3 = (uint8_t)(instruction >> 24);

    // check for function calls and returns
    if ((opcode == VM_ADD || opcode == VM_LDW) && (arg1 == (0x80 | VM_RIP))) {

        // pre-calculate the result
        uint32_t sum = vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3);
        uint32_t result = 0;
        if (opcode == VM_ADD) {
            result = sum;
        } else if (vm_is_addr_valid(vm, sum)) {
            result = vm_load_u32(vm, sum);
        }

        uint32_t rsp = vm->registers[VM_RSP];
        uint32_t rip = vm->registers[VM_RIP];

        if (result != VM_SYSCALL_ADDRESS && vm_is_addr_valid(vm, rsp)) {
            uint32_t top = vm_load_u32(vm, rsp);
            //fprintf(stderr, "ins %X rip 0x%X result 0x%X top 0x%X\n", instruction, rip-4, result, top);

            // check if this is a return (i.e. the result matches the top of the stack)
            if (result == top) {
                debug_callstack_pop(result);
            } else {

                // check if this is a function call
//debug_callstack_print_line(rip-4,false);

                // If the top of the stack contains the next instruction, it's a
                // normal function call. If the top of the stack contains the
                // return address of another function call in our stack, it's a
                // tail call. Either way we push a stack frame.
                if (top == rip) {
                    // The top of the stack contains the address of the next
                    // instruction. This is a normal function call.
                    debug_callstack_push(rip - 4, top, false);
                } else if (debug_stack_has_return(top)) {
                    // The top of the stack contains the same return address as
                    // the current function. This is a tail call.
                    debug_callstack_push(rip - 4, top, true);
                }
            }
        } else if (result != VM_SYSCALL_ADDRESS && debug_callstack_is_empty()) {
            // This is a tail-call from the entry point. We push it so that
            // __start() (or whatever the entry point is called) is in the call
            // stack.
            debug_callstack_push(rip - 4, 0, true);
        }

        //fprintf(stderr, "function is now: 0x%08x ",result); debug_print_location(result, true); fputc('\n', stderr);
    }

    switch (opcode) {
        case VM_ADD:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3);
            break;
        case VM_SUB:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) - vm_parse_mix(vm, arg3);
            break;
        case VM_MUL:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) * vm_parse_mix(vm, arg3);
            break;
        case VM_DIVU: {
            uint32_t divisor = vm_parse_mix(vm, arg3);
            if (divisor == 0) {
                panic("Divide by zero");
            }
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) / divisor;
            break;
        }
        case VM_AND:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) & vm_parse_mix(vm, arg3);
            break;
        case VM_OR:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) | vm_parse_mix(vm, arg3);
            break;
        case VM_SHL: {
            uint32_t shift = vm_parse_mix(vm, arg3);
            if (shift >= 32) {
                panic("Left shift by more than 31");
            }
            vm->registers[vm_parse_register(vm, arg1)] = vm_parse_mix(vm, arg2) << shift;
            break;
        }
        case VM_SHRU: {
            uint32_t shift = vm_parse_mix(vm, arg3);
            if (shift >= 32) {
                panic("Right shift unsigned by more than 31");
            }
            vm->registers[vm_parse_register(vm, arg1)] = vm_parse_mix(vm, arg2) >> shift;
            break;
        }
        case VM_LDW:
            vm->registers[vm_parse_register(vm, arg1)] = vm_load_u32(vm,
                    vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3));
            break;
        case VM_STW:
            //TODO
    //printf("%u %u %u\n",vm_parse_mix(vm, arg2),vm_parse_mix(vm, arg3), (uint32_t)arg3);
            vm_store_u32(vm, vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3),
                    vm_parse_mix(vm, arg1));
            break;
        case VM_LDB:
            vm->registers[vm_parse_register(vm, arg1)] = vm_load_u8(vm,
                    vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3));
            break;
        case VM_STB:
            //TODO
    //printf("%u %u %u\n",vm_parse_mix(vm, arg2),vm_parse_mix(vm, arg3), (uint32_t)arg3);
            vm_store_u8(vm, vm_parse_mix(vm, arg2) + vm_parse_mix(vm, arg3),
                    (uint8_t)vm_parse_mix(vm, arg1));
            break;
        case VM_IMS: {
            size_t dest = vm_parse_register(vm, arg1);
            vm->registers[dest] <<= 16;
            vm->registers[dest] |= (uint32_t)arg2 | ((uint32_t)arg3 << 8);
            break;
        }
        case VM_LTU:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) < vm_parse_mix(vm, arg3);
            break;
        case VM_JZ:
            if (0 == vm_parse_mix(vm, arg1))
                vm->registers[VM_RIP] = (uint32_t)((int32_t)vm->registers[VM_RIP] +
                        ((int32_t)(int16_t)(uint16_t)((uint16_t)arg2 | ((uint16_t)arg3 << 8)) << 2));
            break;
        default:
            panic("Invalid instruction");
            break;
    }
}

static void vm_print_word(uint32_t u) {
    int32_t r = (int32_t)u;
    if (r >= 33 && r <= 126) {
        printf("      %c % 4d", r, r);
    } else {
        printf(" % 11d", r);
    }
    printf(" 0x%08X", r);
}

static const char* vm_instruction_to_string(uint8_t instruction) {
    switch (instruction) {
        case VM_ADD:  return "add";
        case VM_SUB:  return "sub";
        case VM_MUL:  return "mul";
        case VM_DIVU: return "divu";
        case VM_AND:  return "and";
        case VM_OR:   return "or";
        case VM_SHL:  return "shl";
        case VM_SHRU: return "shru";
        case VM_LDW:  return "ldw";
        case VM_STW:  return "stw";
        case VM_LDB:  return "ldb";
        case VM_STB:  return "stb";
        case VM_IMS:  return "ims";
        case VM_LTU:  return "ltu";
        case VM_JZ:   return "jz";
        default: break;
    }
    return "?";
}

static const char* vm_register_to_string(uint8_t r) {
    switch (r) {
        case 0x80: return "r0";
        case 0x81: return "r1";
        case 0x82: return "r2";
        case 0x83: return "r3";
        case 0x84: return "r4";
        case 0x85: return "r5";
        case 0x86: return "r6";
        case 0x87: return "r7";
        case 0x88: return "r8";
        case 0x89: return "r9";
        case 0x8A: return "ra";
        case 0x8B: return "rb";
        case 0x8C: return "rsp";
        case 0x8D: return "rfp";
        case 0x8E: return "rpp";
        case 0x8F: return "rip";
        default: break;
    }
    return "?";
}

#if 0
static const char* vm_syscall_to_string(uint32_t syscall) {
    switch (syscall) {
        // system
        case VM_EXIT: return "exit";
        case VM_TIME: return "time";
        case VM_SPAWN: return "spawn";
        // files
        case VM_OPEN: return "open";
        case VM_CLOSE: return "close";
        case VM_READ: return "read";
        case VM_WRITE: return "write";
        case VM_SEEK: return "seek";
        case VM_TELL: return "tell";
        case VM_TRUNC: return "trunc";
        // directories
        case VM_DOPEN: return "dopen";
        case VM_DCLOSE: return "dclose";
        case VM_DIRENT: return "dread";
        // filesystem
        case VM_STAT: return "stat";
        case VM_RENAME: return "rename";
        case VM_SYMLINK: return "symlink";
        case VM_DELETE: return "delete";
        case VM_CHMOD: return "chmod";
        case VM_MKDIR: return "mkdir";
        case VM_RMDIR: return "rmdir";
        default: break;
    }
    return "?";
}
#endif

static void vm_print_mix(uint8_t b) {
    if (b >= 0x90) {
        printf("%i", (int)b - 256);
    } else if (b < 0x80) {
        printf("%i", (int)b);
    } else {
        fputs(vm_register_to_string(b), stdout);
    }
}

static void vm_print_instruction(uint32_t addr, uint32_t u) {
    uint8_t bytes[4];
    vm_ghost_store_le_u32(bytes, u);
    printf("%02X %02X %02X %02X  ", bytes[0], bytes[1], bytes[2], bytes[3]);
    if (bytes[0] < 0x70 || bytes[0] > 0x7E) {
        fputs("<invalid>", stdout);
        return;
    }
    fputs(vm_instruction_to_string(bytes[0]), stdout);
    putchar(' ');

    switch (bytes[0]) {

        // <reg> <mix> <mix>
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIVU:
        case VM_AND:
        case VM_OR:
        case VM_SHL:
        case VM_SHRU:
        case VM_LDW:
        case VM_LDB:
        case VM_LTU:
            fputs(vm_register_to_string(bytes[1]), stdout);
            putchar(' ');
            vm_print_mix(bytes[2]);
            putchar(' ');
            vm_print_mix(bytes[3]);
            break;

        // <mix> <mix> <mix>
        case VM_STW:
        case VM_STB:
            vm_print_mix(bytes[1]);
            putchar(' ');
            vm_print_mix(bytes[2]);
            putchar(' ');
            vm_print_mix(bytes[3]);
            break;

        case VM_IMS:
            fputs(vm_register_to_string(bytes[1]), stdout);
            printf(" '%02X '%02X", bytes[2], bytes[3]);
            break;

        case VM_JZ:
            vm_print_mix(bytes[1]);
            putchar(' ');
            printf("%i", (int)(int16_t)((uint16_t)bytes[2] | ((uint16_t)bytes[3] << 8)));
            break;

        default:
            vm_ghost_unreachable();
    }

    fputs("    ", stdout);
    fflush(stdout);
    debug_print_location(addr, true);
}

static void vm_print(vm_t* vm) {
    puts("\n");

    for (size_t i = 0; i < 15; ++i) {
        for (size_t i = 0; i < vm_ghost_array_count(vm->recent_addrs); ++i) {
            if (i > 0) {
                fputs("  |  ", stdout);
            }

            // print address
            uint32_t addr = vm->recent_addrs[i] - 7;
            addr &= ~3U;
            printf("0x%08X  ", addr);
            if (!vm_is_addr_valid(vm, addr)) {
                fputs("<invalid>", stdout);
                continue;
            }

            // print ASCII chars
            uint32_t value = vm_load_u32(vm, addr);
            uint8_t bytes[4];
            vm_ghost_store_le_u32(bytes, value);
            for (size_t j = 0; j < 4; ++j) {
                uint8_t c = bytes[j];
                putchar((c >= 33 && c <= 127) ? c : ' ');
            }
            fputs("  ", stdout);

            //
        }
        putchar('\n');
    }

    puts("\n");
    puts("Registers                   |  Stack                               |  Instructions");
    puts("----------------------------|--------------------------------------|---------------------------------------");
    for (size_t i = 0; i < 16; ++i) {

        // print register
        fputs(vm_register_to_string(i | 0x80), stdout);
        if (i < 12)
            putchar(' ');
        vm_print_word(vm->registers[i]);
        fputs("  |", stdout);

        // print stack value
        uint32_t addr = vm->registers[VM_RSP] + (13 - i) * 4;
        if (addr == vm->registers[VM_RSP] && addr == vm->registers[VM_RFP]) {
            fputs("*>", stdout);
        } else if (addr == vm->registers[VM_RSP]) {
            fputs(" >", stdout);
        } else if (addr == vm->registers[VM_RFP]) {
            fputs(" *", stdout);
        } else {
            fputs("  ", stdout);
        }
        printf("0x%06X  ", addr);
        if (!vm_is_addr_aligned(vm, addr)) {
            fputs("  <stack is misaligned>", stdout);
        } else if (!vm_is_addr_valid(vm, addr)) {
            fputs("         <out of range>", stdout);
        } else {
            vm_print_word(vm_load_u32(vm, addr));
        }
        fputs("  | ", stdout);

        // print instruction value
        putchar((i == 3) ? '>' : ' ');
        addr = vm->registers[VM_RIP] + (i - 3) * 4;
        printf("0x%06X  ", addr);
        if (!vm_is_addr_aligned(vm, addr)) {
            fputs("     <rip is misaligned>", stdout);
        // TODO print syscalls
        } else if (!vm_is_addr_valid(vm, addr)) {
            fputs("         <out of range>", stdout);
        } else {
            vm_print_instruction(addr, vm_load_u32(vm, addr));
        }

        putchar('\n');
    }
    fflush(stdout);
}

static void vm_print_stats(vm_t* vm) {
    #ifdef VM_ENABLE_STATS
    size_t pages = 0;
    for (size_t i = 0; i < vm->memory_mask_size; ++i) {
        for (size_t b = 0; b < 8; ++b) {
            if (vm->memory_mask[i] & (1 << b)) {
                ++pages;
            }
        }
    }
    fprintf(stderr, "Total memory usage: %zi bytes, %.2f MiB (%zi pages)\n",
            pages * 4096, pages * 4096 / 1024. / 1024., pages);
    #endif
}

static void vm_loop(vm_t* vm) {
    for (;;) {
        if (!vm->running) {
            vm_print(vm);
            // TODO make a real prompt
            getchar();
        }
        vm_step(vm);
    }
}

int main(int argc, const char* argv[]) {
    terminal_setup();

    common_init();
    debug_init();

    vm_t vm;
static_vm = &vm;
    vm_init(&vm, argc, argv);
    vm_loop(&vm);
    vm_destroy(&vm);

    debug_destroy();
    common_destroy();
}

static void print_callstack(void) {
    debug_callstack_print(static_vm->registers[VM_RIP] - 4);
}
