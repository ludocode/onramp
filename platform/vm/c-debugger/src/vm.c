/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
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

#include <time.h>
#include <inttypes.h>

// TODO put this in ghost
#ifdef _WIN32
    extern char** _environ;
    #define ghost_environ _environ
#else
    extern char** environ;
    #define ghost_environ environ
#endif

#if !vm_ghost_has(vm_ghost_stdint_h)
    #error <stdint.h> is required.
#endif

static void print_callstack(void);

/* TODO should never panic, any error should break into the debugger */
/* TODO actually no, we should only break into the debugger if we were running
 * under it. This should be usable as a normal VM unless set to debug. */
// TODO maybe a separate command-line option, don't run in debugger by default
// but break into it if something goes wrong
vm_ghost_noreturn
static void panic(const char* e) {
    fprintf(stderr, "%s\n", e);
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
#define VM_ADD 0x70  /* add */
#define VM_SUB 0x71  /* subtract */
#define VM_MUL 0x72  /* multiply */
#define VM_DIV 0x73  /* divide */
#define VM_AND 0x74  /* bitwise and */
#define VM_OR  0x75  /* bitwise or */
#define VM_XOR 0x76  /* bitwise xor */
#define VM_ROR 0x77  /* rotate bits right */
#define VM_LDW 0x78  /* load word */
#define VM_STW 0x79  /* store word */
#define VM_LDB 0x7A  /* load byte */
#define VM_STB 0x7B  /* store byte */
#define VM_IMS 0x7C  /* immediate short */
#define VM_CMP 0x7D  /* compare */
#define VM_JZ  0x7E  /* jump if zero */
#define VM_SYS 0x7F  /* interrupt */

/* syscalls */
#define VM_HALT      0x00
#define VM_TIME      0x01
#define VM_SPAWN     0x02
#define VM_FOPEN     0x03
#define VM_FCLOSE    0x04
#define VM_FREAD     0x05
#define VM_FWRITE    0x06
#define VM_FSEEK     0x07
#define VM_FTELL     0x08
#define VM_FTRUNC    0x09
#define VM_DOPEN     0x0A
#define VM_DCLOSE    0x0B
#define VM_DREAD     0x0C
#define VM_STAT      0x0D
#define VM_RENAME    0x0E
#define VM_SYMLINK   0x0F
#define VM_UNLINK    0x10
#define VM_CHMOD     0x11
#define VM_MKDIR     0x12
#define VM_RMDIR     0x13

/* process info table */
#define VM_VERSION 0
#define VM_BREAK 4
#define VM_EXIT 8
#define VM_INPUT 12
#define VM_OUTPUT 16
#define VM_ERROR 20
#define VM_ARGS 24
#define VM_ENVIRON 28
#define VM_WORKDIR 32
#define VM_PIT_SIZE 36

// errors
#define VM_ERR_GENERIC     0xFFFFFFFF
#define VM_ERR_PATH        0xFFFFFFFE
#define VM_ERR_IO          0xFFFFFFFD
#define VM_ERR_UNSUPPORTED 0xFFFFFFFC

/* register and memory value on start */
#define VM_DEFAULT_MEMORY 0xDEADDEAD

/* Files. We offset the file count in order to ensure programs are using them
 * correctly (and not just assuming 1 is stdout for example.) */
#define FILES_COUNT 16
#define FILES_OFFSET (INT_MAX-FILES_COUNT-1)

#define VM_STRACE 0
#if VM_STRACE
    #define strace(...) fprintf(stderr, __VA_ARGS__)
#else
    #define strace(...) ((void)0)
#endif


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

// TODO get rid of all this program_t crap
typedef struct program_t {
   struct program_t* parent;
   uint32_t rfp, rsp, rpp, rip;
} program_t;

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
    program_t* program;
    FILE* files[FILES_COUNT];
    uint32_t recent_addrs[3];
    bool running;
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
        vm_is_addr_valid(vm, addr + size);
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

static void vm_store_u32(vm_t* vm, uint32_t addr, uint32_t value) {
    //printf("storeu32 addr 0x%X rpp 0x%X value 0x%X\n",addr,vm->registers[VM_RPP],value);
    if (!vm_is_addr_valid(vm, addr))
        panic("Invalid address");
    if (!vm_is_addr_aligned(vm, addr))
        panic("Address not aligned");
    //printf("%u %u\n",addr,value);
    vm_ghost_store_le_u32(vm->memory + (addr - vm->memory_base), value);
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
}

static uint8_t vm_load_u8(vm_t* vm, uint32_t addr) {
    if (!vm_is_addr_valid(vm, addr))
        panic("Invalid address");
    return vm->memory[addr - vm->memory_base];
}

static void usage(const char* command) {
    fprintf(stderr, "Usage: %s [vm options] <program> [program options]\n", command);
    fputs("\n", stderr);

    fputs("VM options:\n", stderr);
    // TODO don't need this, use env or other host tools to configure environment
    //fprintf(stderr, "    -e NAME=VAR       define environment variable\n");
    fputs("    -d                start the program paused in the debugger\n", stderr);
    fputs("    -b <location>     add a breakpoint at the given location\n", stderr);
    //fputs("    -m <size>         size of program-accessible address space\n", stderr);
    fputs("    -r <path>         path to root of filesystem\n", stderr);
    fputs("\n", stderr);

    fputs("Breakpoint location syntax:\n", stderr);
    fputs("    TODO\n", stderr);
    fputs("\n", stderr);

    exit(125);
}

static size_t vm_store_string(vm_t* vm, size_t addr, const char* str) {
    size_t size = strlen(str) + 1;
    if (!vm_is_buffer_valid(vm, addr, size)) {
        fputs("ERROR: Out of memory loading string array.\n", stderr);
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

static size_t vm_parse_args(vm_t* vm, int argc, const char* argv[], uint32_t addr) {
    int i;

    // parse vm args
    for (i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "-d")) {
            vm->running = false;
        } else {
            break;
        }
    }

    // parse filename
    if (i == argc) {
        fputs("ERROR: No program filename specified.\n", stderr);
        usage(argv[0]);
    }
    vm->filename = argv[i];

    // load strings
    vm_store_u32(vm, vm->memory_base + VM_ARGS, addr);
    addr = vm_store_string_array(vm, addr, argv + i);
    vm_store_u32(vm, vm->memory_base + VM_ENVIRON, addr);
    addr = vm_store_string_array(vm, addr, vm_ghost_const_cast(const char**, ghost_environ));

    // copy working dir
    vm_store_u32(vm, vm->memory_base + VM_WORKDIR, addr);
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

#if 0
static uint32_t vm_setup_syscall_table(vm_t* vm, uint32_t addr) {

    printf("setup syscall table addr 0x%X membase 0x%X end 0x%X\n",addr,vm->memory_base,vm->memory_base+vm->memory_size);

    vm->syscall_table = addr;
    uint32_t syscall_count = 128; // TODO
    if (!vm_is_buffer_valid(vm, addr, syscall_count * 4)) {
        panic("Out of memory setting up syscall table");
    }
    uint32_t ret = addr + syscall_count * 4;

    //printf("setting up syscalls\n");
    #define SETUP_SYSCALL(x) vm_store_u32(vm, addr + x, vm->syscall_addr + x);
    SETUP_SYSCALL(VM_HALT)
    SETUP_SYSCALL(VM_INPUT)
    SETUP_SYSCALL(VM_OUTPUT)
    SETUP_SYSCALL(VM_ERROR)
    #undef SETUP_SYSCALL
    //printf("done setting up syscalls\n");

    return ret;
}
#endif

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

    vm->memory = vm_ghost_alloc_array(uint8_t, vm->memory_size);
    for (size_t i = 0; i + 3 < vm->memory_size; i += 4) {
        vm_store_u32(vm, vm->memory_base + i, VM_DEFAULT_MEMORY);
    }

    /* parse arguments and environment variables */
    uint32_t addr = vm->memory_base + VM_PIT_SIZE;
    addr = vm_parse_args(vm, argc, argv, addr);

    /* setup files */
    vm->files[0] = stdin;
    vm->files[1] = stdout;
    vm->files[2] = stderr;
    vm_store_u32(vm, vm->memory_base + VM_INPUT, FILES_OFFSET);
    vm_store_u32(vm, vm->memory_base + VM_OUTPUT, FILES_OFFSET + 1);
    vm_store_u32(vm, vm->memory_base + VM_ERROR, FILES_OFFSET + 2);

    /* add a halt instruction */
    vm_store_u32(vm, addr, 0x0000007F);
    vm_store_u32(vm, vm->memory_base + VM_EXIT, addr);
    addr += 4;

    /* add some padding so a user isn't confused when viewing their program in
     * the debugger */
    for (size_t i = 0; i < 32; ++i) {
        vm_store_u32(vm, addr, VM_DEFAULT_MEMORY);
        addr += 4;
    }

    /* round up the program address to a multiple of 0x10000 to make it easier
     * to debug */
    addr = (addr + 0x10000 - 1) & ~(0x10000 - 1);

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

    /* set up the rest of the process info table */
    vm_store_u32(vm, vm->memory_base + VM_VERSION, 0);
    vm_store_u32(vm, vm->memory_base + VM_BREAK, addr);

    // push the halt syscall as the _start return address
    // TODO halt address is now in the PIT, but we may put it back on the stack later
    //end -= 4;
    //vm_store_u32(vm, end, vm->syscall_addr);

    vm->registers[0] = vm->memory_base;
    for (size_t i = 1; i <= VM_RFP; ++i)
        vm->registers[i] = VM_DEFAULT_MEMORY;
    vm->registers[VM_RSP] = end;
    vm->registers[VM_RPP] = start;
    vm->registers[VM_RIP] = start;

    vm->recent_addrs[0] = start;
    vm->recent_addrs[1] = addr;
    vm->recent_addrs[2] = end;

    fclose(file);
}

static void vm_destroy(vm_t* vm) {
    // TODO estimate memory usage, maybe by a bitmap of 4k pages accessed
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
static FILE* vm_file(vm_t* vm, uint32_t handle) {
    handle -= FILES_OFFSET;
    if (handle >= (uint32_t)vm_ghost_array_count(vm->files))
        panic("I/O handle is invalid");
    FILE* file = vm->files[handle];
    if (file == vm_ghost_null)
        panic("I/O handle is not open");
    return file;
}

static uint32_t vm_halt(vm_t* vm) {
    // TODO pause debugger
    strace("sys halt() %i\n", vm->registers[0]);
    exit(vm_parse_mix(vm, vm->registers[0]));
    return VM_ERR_GENERIC;
}

static uint32_t vm_time(vm_t* vm) {
    struct timespec time;
    if (0 != clock_gettime(CLOCK_REALTIME, &time)) {
        return VM_ERR_UNSUPPORTED;
    }
    strace("sys time() %" PRIi64 " s %u ns\n", (uint64_t)time.tv_sec, (unsigned)time.tv_nsec);

    uint32_t addr = vm->registers[0];
    vm_store_u32(vm, addr, (uint32_t)time.tv_sec);
    vm_store_u32(vm, addr, (uint32_t)((uint64_t)time.tv_nsec >> 32));
    vm_store_u32(vm, addr, time.tv_nsec);
    return 0;
}

static uint32_t vm_spawn(vm_t* vm) {
    strace("sys spawn()\n");
    panic("TODO spawn syscall not yet implemented");
}

static uint32_t vm_fopen(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    uint32_t mode = vm->registers[1];

    // find a free handle (not the standard streams 0,1,2)
    uint32_t file_index = UINT32_MAX;
    for (size_t i = 3; i < vm_ghost_array_count(vm->files); ++i) {
        if (vm->files[i] == vm_ghost_null) {
            file_index = i;
            break;
        }
    }
    if (file_index == UINT32_MAX) {
        panic("No free I/O handles");
    }

    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));

    // open it
    vm->files[file_index] = fopen(full_path, mode ? "a+b" : "rb");
    //printf("OPENING %s %zi\n",full_path,(size_t)vm->files[file_index]);
    if (vm->files[file_index] == vm_ghost_null) {
        strace("sys fopen() path %s mode %i failed.\n", full_path, mode);
        return VM_ERR_PATH;
    }
    strace("sys fopen() path %s mode %u returning handle 0x%x\n", full_path, mode, file_index + FILES_OFFSET);

    // if writeable, seek to the beginning
    if (mode) {
        fseek(vm->files[file_index], 0, SEEK_SET);
    }

    return file_index + FILES_OFFSET;
}

static uint32_t vm_fclose(vm_t* vm) {
    uint32_t handle = vm->registers[0];
    FILE* file = vm_file(vm, handle);
    strace("sys fclose() handle 0x%x\n", handle);
    if (file == vm_ghost_null) {
        panic("File is not open.");
    }
    if (file == stdin || file == stdout || file == stderr) {
        panic("Cannot close standard streams.");
    }
    fclose(file);
    vm->files[handle - FILES_OFFSET] = vm_ghost_null;
    return 0;
}

static uint32_t vm_fread(vm_t* vm) {
    FILE* file = vm_file(vm, vm->registers[0]);
    uint32_t addr = vm->registers[1];
    uint32_t count = vm->registers[2];
    strace("sys fread() handle 0x%x addr 0x%x count %u\n", vm->registers[0], addr, count);

    if (count == 0) {
        // nothing to do, addr does not need to be valid
        return 0;
    }
    if (!vm_is_buffer_valid(vm, addr, count)) {
        panic("ERROR: Invalid buffer given to syscall fread.");
    }

    size_t ret = fread(vm->memory + (addr - vm->memory_base), 1, count, file);
    if (ret == 0) {
        if (feof(file))
            return 0;
        return VM_ERR_IO;
    }
    return (uint32_t)ret;
}

static uint32_t vm_fwrite(vm_t* vm) {
    FILE* file = vm_file(vm, vm->registers[0]);
    uint32_t addr = vm->registers[1];
    uint32_t count = vm->registers[2];
    strace("sys fwrite() handle 0x%x addr 0x%x count %u\n", vm->registers[0], addr, count);

    if (count == 0) {
        // nothing to do, addr does not need to be valid
        return 0;
    }
    if (!vm_is_buffer_valid(vm, addr, count)) {
        panic("ERROR: Invalid buffer given to syscall fwrite.");
    }

    uint8_t* buffer = vm->memory + (addr - vm->memory_base);
    size_t ret = fwrite(buffer, 1, count, file);
    if (ret == 0) {
        panic("Error writing file!");
    }

    // We need to flush to ensure this doesn't interfere with our debugger.
    // TODO only flush when debugger NOT running
    // TODO why not running? shouldn't we only flush when running?
    /*if (file == stdout || file == stderr) {
        fflush(file);
    }*/

    return (uint32_t)ret;
}

static uint32_t vm_fseek(vm_t* vm) {
    FILE* file = vm_file(vm, vm->registers[0]);
    uint32_t base = vm->registers[1];
    int64_t offset = (int64_t)((uint64_t)vm->registers[2] | ((uint64_t)vm->registers[3] << 32));
    strace("sys fseek() handle 0x%x base %u offset %" PRIi64 "\n", vm->registers[0], base, offset);

    if (base > 2) {
        fatal("Invalid base given to syscall fseek.");
    }

    fseek(file, offset,
            base == 0 ? SEEK_SET : base == 1 ? SEEK_CUR : SEEK_END);
    return 0;
}

static uint32_t vm_ftell(vm_t* vm) {
    FILE* file = vm_file(vm, vm->registers[0]);
    uint32_t addr = vm->registers[1];
    long pos = ftell(file);
    strace("sys ftell() handle 0x%x position %" PRIu64 "\n", vm->registers[0], (uint64_t)pos);
    vm_store_u32(vm, addr, (uint32_t)pos);
    vm_store_u32(vm, addr + 4, (uint32_t)(pos >> 32));
    return 0;
}

static uint32_t vm_ftrunc(vm_t* vm) {
    FILE* file = vm_file(vm, vm->registers[0]);
    fflush(file);
    uint64_t length = (uint64_t)vm->registers[1] | ((uint64_t)vm->registers[2] << 32);
    strace("sys ftrunc() handle 0x%x length %" PRIu64 "\n", vm->registers[0], (uint64_t)length);
    if (0 == ftruncate(fileno(file), length))
        return 0;
    // TODO error codes
    return VM_ERR_GENERIC;
}

static uint32_t vm_dopen(vm_t* vm) {
    panic("TODO dopen syscall not yet implemented");
}

static uint32_t vm_dclose(vm_t* vm) {
    panic("TODO dclose syscall not yet implemented");
}

static uint32_t vm_dread(vm_t* vm) {
    panic("TODO dread syscall not yet implemented");
}

static uint32_t vm_stat(vm_t* vm) {
    panic("TODO stat syscall not yet implemented");
}

static uint32_t vm_rename(vm_t* vm) {
    panic("TODO rename syscall not yet implemented");
}

static uint32_t vm_symlink(vm_t* vm) {
    panic("TODO symlink syscall not yet implemented");
}

static uint32_t vm_unlink(vm_t* vm) {
    uint32_t path_addr = vm->registers[0];
    if (!vm_is_string_valid(vm, path_addr)) {
        fputs("ERROR: Invalid path.\n", stderr);
        exit(125);
    }
    const char* full_path = (const char*)(vm->memory + (path_addr - vm->memory_base));
    //printf("VM: Opening %s for %s\n", full_path, mode ? "writing" : "reading");
    if (0 == unlink(full_path))
        return 0;
    // TODO correct error codes
    return VM_ERR_GENERIC;
}

static int vm_chmod(vm_t* vm) {
    // TODO this should take a path, not a file descriptor
    FILE* file = vm_file(vm, vm->registers[0]);
    mode_t mode = vm->registers[1];
    if (mode != 0644 && mode != 0755) {
        panic("Invalid chmod mode");
    }
    fchmod(fileno(file), mode);
    return 0;
}

static uint32_t vm_mkdir(vm_t* vm) {
    panic("TODO mkdir syscall not yet implemented");
}

static uint32_t vm_rmdir(vm_t* vm) {
    panic("TODO rmdir syscall not yet implemented");
}

vm_ghost_noinline
static void vm_sys(vm_t* vm, uint8_t syscall_number, uint8_t arg1, uint8_t arg2) {
    //fprintf(stderr, "vm_sys %u %u %u\n", syscall_number, arg1, arg2);
    if (arg1 != 0 || arg2 != 0) {
        panic("Extra arguments to syscall must be 0");
    }

    int ret = 0;
    switch (syscall_number) {
        // misc
        case VM_HALT:      ret = vm_halt(vm); break;
        case VM_TIME:      ret = vm_time(vm); break;
        case VM_SPAWN:     ret = vm_spawn(vm); break;
        // file
        case VM_FOPEN:     ret = vm_fopen(vm); break;
        case VM_FCLOSE:    ret = vm_fclose(vm); break;
        case VM_FREAD:     ret = vm_fread(vm); break;
        case VM_FWRITE:    ret = vm_fwrite(vm); /*break
                            TODO syscall fwrite currently doesn't set r0, we need to clean up some code first */
                            return;
        case VM_FSEEK:     ret = vm_fseek(vm); break;
        case VM_FTELL:     ret = vm_ftell(vm); break;
        case VM_FTRUNC:    ret = vm_ftrunc(vm); break;
        // directory
        case VM_DOPEN:     ret = vm_dopen(vm); break;
        case VM_DCLOSE:    ret = vm_dclose(vm); break;
        case VM_DREAD:     ret = vm_dread(vm); break;
        // filesystem
        case VM_STAT:      ret = vm_stat(vm); break;
        case VM_RENAME:    ret = vm_rename(vm); break;
        case VM_SYMLINK:   ret = vm_symlink(vm); break;
        case VM_UNLINK:    ret = vm_unlink(vm); break;
        case VM_CHMOD:     ret = vm_chmod(vm); break;
        case VM_MKDIR:     ret = vm_mkdir(vm); break;
        case VM_RMDIR:     ret = vm_rmdir(vm); break;
        default:
            panic("Unrecognized syscall");
    }

    vm->registers[0] = ret;
}

#if 0
//ghost_noinline
static bool vm_syscall(vm_t* vm) {
    uint32_t rip = vm->registers[VM_RIP];
    //printf("TRYING SYSCALL 0x%x 0x%x 0x%x\n", rip, vm->syscall_addr,rip - vm->syscall_addr);
    switch (rip - vm->syscall_addr) {
        case VM_OUTPUT: {
            uint32_t buffer = vm->registers[0];
            uint32_t count = vm->registers[1];
            //printf("buffer 0x%X count %u\n",buffer,count);
            if (!vm_is_buffer_valid(vm, buffer, count)) {
                panic("Invalid buffer for output");
                return false;
            }
            fwrite(vm->memory + buffer - vm->memory_base, 1, count, stdout);
            break;
        }

        case VM_HALT:
            exit(vm->registers[0]);
            break;

        case VM_INPUT:
        case VM_ERROR:
            printf("SYSCALL %s\n", vm_syscall_to_string(rip - vm->syscall_addr));
            break;
        default:
            return false;
    }

    // Registers don't need to be preserved by syscalls (since they may be
    // normal functions.) To ensure programs don't rely on this we clear all
    // caller-saved registers.
    for (int i = 0x0; i <= 0xB; ++i) {
        vm->registers[i] = VM_DEFAULT_MEMORY;
    }

    // jump to return address
    vm->registers[VM_RIP] = vm_load_u32(vm, vm->registers[VM_RSP]);
    return true;
}
#endif

static void vm_step(vm_t* vm) {
    /*
    if (!vm_is_addr_valid(vm, vm->registers[VM_RIP]))
        if (vm_syscall(vm))
            return;
            */

    uint32_t instruction = vm_load_u32(vm, vm->registers[VM_RIP]);
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

        if (vm_is_addr_valid(vm, rsp)) {
            uint32_t top = vm_load_u32(vm, rsp);
            //printf("ins %X rip 0x%X result 0x%X top 0x%X\n", instruction, rip-4, result, top);

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
                    // The top of the stack contains the return address of a parent
                    // function call in our stack. This is a tail call.
                    debug_callstack_push(rip - 4, top, true);
                }
            }
        }
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
        case VM_DIV:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) / vm_parse_mix(vm, arg3);
            break;
        case VM_AND:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) & vm_parse_mix(vm, arg3);
            break;
        case VM_OR:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) | vm_parse_mix(vm, arg3);
            break;
        case VM_XOR:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_parse_mix(vm, arg2) ^ vm_parse_mix(vm, arg3);
            break;
        case VM_ROR:
            vm->registers[vm_parse_register(vm, arg1)] =
                    vm_ghost_rotr_u32(vm_parse_mix(vm, arg2), vm_parse_mix(vm, arg3));
            break;
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
        case VM_CMP: {
            size_t left = vm_parse_mix(vm, arg2);
            size_t right = vm_parse_mix(vm, arg3);
            vm->registers[vm_parse_register(vm, arg1)] =
                    (left < right) ? -1 : (left > right) ? 1 : 0;
            break;
        }
        case VM_JZ:
            if (0 == vm_parse_mix(vm, arg1))
                vm->registers[VM_RIP] = (uint32_t)((int32_t)vm->registers[VM_RIP] +
                        ((int32_t)(int16_t)(uint16_t)((uint16_t)arg2 | ((uint16_t)arg3 << 8)) << 2));
            break;
        case VM_SYS:
            vm_sys(vm, arg1, arg2, arg3);
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
        case VM_ADD: return "add";
        case VM_SUB: return "sub";
        case VM_MUL: return "mul";
        case VM_DIV: return "div";
        case VM_AND: return "and";
        case VM_OR:  return "or";
        case VM_XOR: return "xor";
        case VM_ROR: return "ror";
        case VM_LDW: return "ldw";
        case VM_STW: return "stw";
        case VM_LDB: return "ldb";
        case VM_STB: return "stb";
        case VM_IMS: return "ims";
        case VM_CMP: return "cmp";
        case VM_JZ:  return "jz";
        case VM_SYS: return "sys";
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

static const char* vm_syscall_to_string(uint32_t syscall) {
    switch (syscall) {
        // system
        case VM_HALT: return "halt";
        case VM_TIME: return "time";
        case VM_SPAWN: return "spawn";
        // files
        case VM_FOPEN: return "fopen";
        case VM_FCLOSE: return "fclose";
        case VM_FREAD: return "fread";
        case VM_FWRITE: return "fwrite";
        case VM_FSEEK: return "fseek";
        case VM_FTELL: return "ftell";
        case VM_FTRUNC: return "ftrunc";
        // directories
        case VM_DOPEN: return "dopen";
        case VM_DCLOSE: return "dclose";
        case VM_DREAD: return "dread";
        // filesystem
        case VM_STAT: return "stat";
        case VM_RENAME: return "rename";
        case VM_SYMLINK: return "symlink";
        case VM_UNLINK: return "unlink";
        case VM_CHMOD: return "chmod";
        case VM_MKDIR: return "mkdir";
        case VM_RMDIR: return "rmdir";
        default: break;
    }
    return "?";
}

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
    if (bytes[0] < 0x70 || bytes[0] > 0x7F) {
        fputs("<invalid>", stdout);
        return;
    }
    fputs(vm_instruction_to_string(bytes[0]), stdout);
    putchar(' ');

    switch (bytes[0]) {
        // math-like, i.e. <reg> <mix> <mix>
        case VM_ADD: //fallthrough
        case VM_SUB: //fallthrough
        case VM_MUL: //fallthrough
        case VM_DIV: //fallthrough
        case VM_AND: //fallthrough
        case VM_OR:  //fallthrough
        case VM_XOR: //fallthrough
        case VM_ROR: //fallthrough
        case VM_LDW: //fallthrough
        case VM_LDB: //fallthrough
        case VM_CMP:
            fputs(vm_register_to_string(bytes[1]), stdout);
            putchar(' ');
            vm_print_mix(bytes[2]);
            putchar(' ');
            vm_print_mix(bytes[3]);
            break;

        case VM_STW: //fallthrough
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

        case VM_SYS:
            fputs(vm_syscall_to_string(bytes[1]), stdout);
            printf(" '%02X '%02X", bytes[2], bytes[3]);
            break;

        default:
            vm_ghost_unreachable();
    }

    fputs("    ", stdout);
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
    // We fully buffer output so as not to flicker when animating our debug info.
    setvbuf(stdout, NULL, BUFSIZ, _IOFBF);

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
