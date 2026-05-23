// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// This is mostly a translation of test/vm/files/dirent.oe.ohx to C with some
// additional checks.

int main(void) {
    DIR* dir = opendir("test/vm/testdata/dir");
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    bool files_found[6] = {};
    const char* files_expected[6] = {
        ".",
        "..",
        "alice",
        "bob",
        "carol",
        "dave",
    };
    size_t count = 0;
    const char* order[6];

    for (;;) {
        errno = 0;
        struct dirent* dirent = readdir(dir);
        if (!dirent) {
            if (errno != 0) {
                perror("readdir");
                exit(2);
            }
            break;
        }

        bool found = false;
        for (size_t i = 0; i < sizeof(files_expected)/sizeof(*files_expected); ++i) {
            if (0 == strcmp(files_expected[i], dirent->d_name)) {
                if (files_found[i]) {
                    fprintf(stderr, "Duplicate filename: \"%s\"", dirent->d_name);
                    exit(3);
                }
                files_found[i] = true;
                order[count++] = files_expected[i];
                found = true;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Unexpected filename: \"%s\"", dirent->d_name);
            exit(3);
        }
    }

    // Unlike test/vm/files/dirent.oe.ohx, the "." and ".." must be present
    // because the libc synthesizes them.
    for (size_t i = 0; i < sizeof(files_expected)/sizeof(*files_expected); ++i) {
        if (!files_found[i]) {
            fprintf(stderr, "Missing filename: \"%s\"", files_expected[i]);
            exit(4);
        }
    }

    closedir(dir);

    // Read it again, make sure the files remain in the same order
    dir = opendir("test/vm/testdata/dir");
    if (!dir) {
        perror("opendir (repeat)");
        exit(5);
    }
    size_t count2 = 0;
    for (;; ++count2) {
        errno = 0;
        struct dirent* dirent = readdir(dir);
        if (!dirent) {
            if (errno != 0) {
                perror("readdir (repeat)");
                exit(7);
            }
            break;
        }

        if (count2 == count) {
            fprintf(stderr, "Too many files on second read\n");
            exit(6);
        }

        if (0 != strcmp(dirent->d_name, order[count2])) {
            fprintf(stderr,
                    "Files out of order on second read! Expected \"%s\" at %zu, got \"%s\"\n",
                    order[count2], count2, dirent->d_name);
            exit(8);
        }
    }

    if (count2 != count) {
        fprintf(stderr, "Not enough files on second read\n");
        exit(9);
    }

    closedir(dir);
}
