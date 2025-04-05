// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <string.h>
#include <stdlib.h>

// We support the POSIX global environ variable, including the ability to
// replace it.
extern char** environ;

int main(int argc, char** argv, char** envp) {

    // We support the extension to main() to get the environment. Make sure the
    // given evp matches the global variable environ.
    if (envp != environ) return 1;

    // Make sure it's not null. (Even if the VM gave a null pointer, the libc
    // should fix it.)
    if (envp == NULL) return 2;

    // Make sure it's null terminated and any strings are also null-terminated
    for (; *envp; ++envp) {
        char* str = *envp;
        for (; *str; ++str) {}
    }

    // Fully replace the environment
    char* new_environ[] = {
        "ALICE=alice",
        "BOB=bob",
        "CARL=carl",
        "DAVE=dave",
        NULL
    };
    environ = new_environ;

    // Make sure it contains what we expect
    if (NULL == getenv("ALICE")) return 3;
    if (NULL == getenv("BOB")) return 4;
    if (NULL == getenv("CARL")) return 5;
    if (NULL == getenv("DAVE")) return 6;
    if (NULL != getenv("EVE")) return 7;
    if (0 != strcmp("alice", getenv("ALICE"))) return 8;
    if (0 != strcmp("bob", getenv("BOB"))) return 9;
    if (0 != strcmp("carl", getenv("CARL"))) return 10;
    if (0 != strcmp("dave", getenv("DAVE"))) return 11;

    // Make sure no old stuff is in there
    if (NULL != getenv("PATH")) return 12;
    if (NULL != getenv("HOME")) return 13;
    if (NULL != getenv("USER")) return 14;

    // Make sure it's still editable from our old pointer
    new_environ[1] = "EVE=eve";

    // Make sure it still works
    if (NULL == getenv("ALICE")) return 15;
    if (NULL != getenv("BOB")) return 16;
    if (NULL == getenv("CARL")) return 17;
    if (NULL == getenv("DAVE")) return 18;
    if (NULL == getenv("EVE")) return 19;
    if (0 != strcmp("alice", getenv("ALICE"))) return 20;
    if (0 != strcmp("eve", getenv("EVE"))) return 21;
    if (0 != strcmp("carl", getenv("CARL"))) return 22;
    if (0 != strcmp("dave", getenv("DAVE"))) return 23;

    // Make sure putenv() allocates a new environment (and doesn't try to free
    // or resize the one we set)
    putenv("FRED=fred");
    if (environ == new_environ) return 24;
    if (NULL == getenv("ALICE")) return 25;
    if (NULL != getenv("BOB")) return 26;
    if (NULL == getenv("CARL")) return 27;
    if (NULL == getenv("DAVE")) return 28;
    if (NULL == getenv("EVE")) return 29;
    if (NULL == getenv("FRED")) return 30;

    // Make sure clearenv() works
    clearenv();
    if (NULL != environ[0]) return 31;
    if (NULL != getenv("ALICE")) return 32;
}
