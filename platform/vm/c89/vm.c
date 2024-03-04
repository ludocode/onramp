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
 * This is a simple implementation of the Onramp virtual machine in ANSI C.
 *
 * It does not load debug info or do any debugging. It otherwise performs all
 * required checks and many optional checks and implements all system calls.
 *
 * If you're trying to port Onramp to an old system that only has a C89
 * compiler, this is probably the best place to start.
 */



/*
 * Portability
 */

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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



/*
 * Globals
 */

#define VM_MEMORY_SIZE (16 * 1024 * 1024) /* 16 MB */
#define VM_MAX_FILES 16
#define VM_MAX_DIRECTORIES 16

static uint8_t vm_memory[VM_MEMORY_SIZE];
static FILE* vm_files[VM_MAX_FILES];
//static uint32_t vm_directories[VM_MAX_DIRECTORIES];
static uint32_t vm_registers[16];

/* array indices of named registers */
#define VM_RSP 0xC  /* stack pointer */
#define VM_RFP 0xD  /* frame pointer */
#define VM_RPP 0xE  /* program pointer */
#define VM_RIP 0xF  /* instruction pointer */



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
    fprintf(stderr, "VM options:\n");
    fprintf(stderr, "    -e NAME=VAR       define environment variable (no vars are passed by default)\n");
    exit(125);
}

#define vm_check(expr, msg) (!(expr) ? vm_panic(msg) : (void)0)

#define vm_check_aligned(addr) \
    vm_check(((addr) & 3) == 0, "Misaligned address")

/* Our virtual memory starts at 0 but we disable access to the first word to
 * prevent null pointer dereferences. */
#define vm_check_valid(addr) \
    vm_check((addr) >= 4 && (addr) < VM_MEMORY_SIZE, "Address out of bounds")

#define vm_check_file(handle) \
    vm_check((uint32_t)(handle) < VM_MAX_FILES && vm_files[(handle)] != NULL, \
            "Invalid file descriptor")

#define vm_check_directory(dd) \
    vm_check((uint32_t)(dd) < VM_MAX_FILES && vm_directories[(dd)] != NULL, \
            "Invalid directory descriptor")



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
    vm_check_file(handle);
    return vm_files[handle];
}

static void vm_check_string(uint32_t addr) {
    while (vm_load_u8(addr++) != '\0') {}
}

static void vm_check_buffer(uint32_t addr, uint32_t size) {
    vm_check(size > 0, "Invalid size of zero for syscall buffer");
    vm_check_valid(addr);
    vm_check_valid(addr + size - 1);
}



/*
 * Initialization
 */

static uint32_t vm_load_program(uint32_t start, const char* filename) {
    FILE* file;
    uint32_t addr;

    /* Read the entire program into memory */
    file = fopen(filename, "rb");
    if (file == NULL) {
        vm_panic("Couldn't open program");
    }
    addr = start;
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

    /* Make sure there's still at least some room for heap and stack */
    if (VM_MEMORY_SIZE - addr < 32 * 1024) {
        vm_panic("Program is too big.");
    }

    /* Check the format indicator */
    if (vm_load_u32(start) != 0x726E4F7E ||
            vm_load_u32(start + 4) != 0x706D617E ||
            vm_load_u32(start + 8) != 0x2020207E)
    {
        //printf("%x\n", vm_load_u32(start));
        fprintf(stderr, "WARNING: Program does not start with \"~Onr~amp~   \" format indicator.\n");
    }

    /* Setup registers */
    memset(vm_registers, 0, sizeof(vm_registers));
    vm_registers[0] = 4;
    vm_registers[1] = 0; /* TODO command-line args */
    vm_registers[2] = 0; /* TODO env vars */
    vm_registers[VM_RFP] = VM_MEMORY_SIZE;
    vm_registers[VM_RSP] = VM_MEMORY_SIZE;
    vm_registers[VM_RPP] = start;
    vm_registers[VM_RIP] = start;

    return addr;
}

static void vm_init(int argc, const char* argv[]) {
    const char* filename = NULL;
    int i;
    uint32_t address, process_info_address, halt_address;

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

    /* reserve space for process info table */
    address = 4;
    process_info_address = address;
    address += 36;

    /* write halt instruction */
    halt_address = address;
    vm_store_u32(address, 0x0000007f);
    address += 4;

    /* configure process info table */
    vm_store_u32(process_info_address + 8, halt_address);
    vm_store_u32(process_info_address + 12, 0); /* stdin */
    vm_store_u32(process_info_address + 16, 1); /* stdout */
    vm_store_u32(process_info_address + 20, 2); /* stderr */

    /* TODO copy command-line args into memory */
    vm_store_u32(process_info_address + 24, 0); /* args */
    vm_store_u32(process_info_address + 28, 0); /* environ */

    /* TODO copy workdir into memory */

    /* files */
    vm_files[0] = stdin;
    vm_files[1] = stdout;
    vm_files[2] = stderr;

    uint32_t break_address = vm_load_program(address, filename);
    vm_store_u32(process_info_address + 4, break_address);
}



/*
 * System Calls
 */

static void vm_halt(void) {
    exit(vm_parse_mix(vm_registers[0]));
}

static void vm_open(void) {
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
        if (vm_files[i] == NULL) {
            handle = i;
            break;
        }
    }
    vm_check(handle != UINT32_MAX, "No free file descriptors");

    /* open it */
    vm_files[handle] = fopen(path, mode ? "wb" : "rb");
    if (vm_files[handle] == NULL) {
        /* TODO right now there's no way to tell whether a file exists. */
        vm_panic("File does not exist.");
    }

    vm_registers[0] = handle;
}

static void vm_close(void) {
    uint32_t handle = vm_registers[0];
    vm_check_file(handle);
    vm_check(handle > 2, "Cannot close standard streams.");
    fclose(vm_files[handle]);
    vm_files[handle] = NULL;
}

static void vm_read(void) {
    FILE* file = vm_file(vm_registers[0]);
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];
    size_t ret;

    vm_check_buffer(addr, count);
    ret = fread(vm_memory + addr, 1, count, file);
    if (ret == 0 && !feof(file)) {
        vm_panic("Error reading file!");
    }

    vm_registers[0] = (uint32_t)ret;
}

static void vm_write(void) {
    FILE* file = vm_file(vm_registers[0]);
    uint32_t addr = vm_registers[1];
    uint32_t count = vm_registers[2];
    size_t ret;

    vm_check_buffer(addr, count);
    while (count > 0) {
        ret = fwrite(vm_memory + addr, 1, count, file);
        if (ret == 0) {
            vm_panic("Error writing file!");
        }
        addr += ret;
        count -= ret;
    }
}

static void vm_sys(uint8_t syscall) {
    /*printf("%u %u\n",arg1,arg2);*/

    switch (syscall) {
        case 0x0: /* halt */
            vm_halt();
            return;
        case 0x10: /* open */
            vm_open();
            return;
        case 0x11: /* close */
            vm_close();
            return;
        case 0x12: /* read */
            vm_read();
            return;
        case 0x13: /* write */
            vm_write();
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
        case 0x76:   *reg = mix1 ^ mix2;   goto next;  /* xor */
        case 0x77: {                                   /* ror */
            mix2 &= 0x1Fu;
            *reg = (mix1 >> mix2) | (mix1 << ((-mix2) & 0x1Fu));
            goto next;
        }
        case 0x78:   *reg = vm_load_u32(mix1 + mix2);   goto next;  /* ldw */
        case 0x7A:   *reg = vm_load_u8 (mix1 + mix2);   goto next;  /* ldb */
        case 0x7D:                                                  /* cmpu */
            *reg = (mix1 < mix2) ? -1 : (mix1 > mix2) ? 1 : 0;
            goto next;
        default:
            break;
    }

    vm_panic("Invalid instruction");
}

int main(int argc, const char** argv) {
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
