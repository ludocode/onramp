/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

/* TODO we don't have a proper libo test harness yet. For now:
make -C test/libo/1-opc/ build-native && cc -g -Icore/libo/1-opc/include test/libo/1-opc/test_otable.c output/test/libo-1-opc/libo.a -o /tmp/otable && /tmp/otable
make -C test/libo/1-opc/ build-native && cc -fsanitize=address -g -Icore/libo/1-opc/include test/libo/1-opc/test_otable.c output/test/libo-1-opc/libo.a -o /tmp/otable && /tmp/otable
*/

#include "libo-otable.h"

#include <stdlib.h>
#include <stdio.h>

#define test_assert(x) \
    if (!(x)) {printf("failed line %i: %s\n", __LINE__, #x); exit(1);}

char ascii[128];

static void init_ascii(void) {
    for (size_t i = 0; i < sizeof(ascii) / sizeof(*ascii); ++i) {
        ascii[i] = i;
    }
}

static void test_simple() {
    otable_t* otable = otable_new();

    otable_put(otable, ascii + 'a', 'a');
    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(!otable_contains(otable, ascii + 'b', 'b'));
    test_assert(!otable_contains(otable, ascii + 'c', 'c'));
    test_assert(!otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));

    otable_put(otable, ascii + 'b', 'b');
    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(otable_contains(otable, ascii + 'b', 'b'));
    test_assert(!otable_contains(otable, ascii + 'c', 'c'));
    test_assert(!otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));

    otable_put(otable, ascii + 'c', 'c');
    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(otable_contains(otable, ascii + 'b', 'b'));
    test_assert(otable_contains(otable, ascii + 'c', 'c'));
    test_assert(!otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));

    otable_put(otable, ascii + 'd', 'd');
    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(otable_contains(otable, ascii + 'b', 'b'));
    test_assert(otable_contains(otable, ascii + 'c', 'c'));
    test_assert(otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));

    otable_delete(otable);
}

static void test_simple_all_collision() {
    otable_t* otable = otable_new();

    // use hash 0 for all. should still work, degenerates into an array set

    otable_put(otable, ascii + 'a', 0);
    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(!otable_contains(otable, ascii + 'b', 0));
    test_assert(!otable_contains(otable, ascii + 'c', 0));
    test_assert(!otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));

    otable_put(otable, ascii + 'b', 0);
    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(otable_contains(otable, ascii + 'b', 0));
    test_assert(!otable_contains(otable, ascii + 'c', 0));
    test_assert(!otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));

    otable_put(otable, ascii + 'c', 0);
    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(otable_contains(otable, ascii + 'b', 0));
    test_assert(otable_contains(otable, ascii + 'c', 0));
    test_assert(!otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));

    otable_put(otable, ascii + 'd', 0);
    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(otable_contains(otable, ascii + 'b', 0));
    test_assert(otable_contains(otable, ascii + 'c', 0));
    test_assert(otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));

    otable_delete(otable);
}

static void test_remove() {
    otable_t* otable = otable_new();

    otable_put(otable, ascii + 'a', 'a');
    otable_put(otable, ascii + 'b', 'b');
    otable_put(otable, ascii + 'c', 'c');
    otable_put(otable, ascii + 'd', 'd');
    otable_put(otable, ascii + 'e', 'e');
    otable_put(otable, ascii + 'f', 'f');
    otable_put(otable, ascii + 'g', 'g');
    otable_put(otable, ascii + 'h', 'h');
    otable_put(otable, ascii + 'i', 'i');
    test_assert(9 == otable_count(otable));

    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(otable_contains(otable, ascii + 'b', 'b'));
    test_assert(otable_contains(otable, ascii + 'c', 'c'));
    test_assert(otable_contains(otable, ascii + 'd', 'd'));
    test_assert(otable_contains(otable, ascii + 'e', 'e'));
    test_assert(otable_contains(otable, ascii + 'f', 'f'));
    test_assert(otable_contains(otable, ascii + 'g', 'g'));
    test_assert(otable_contains(otable, ascii + 'h', 'h'));
    test_assert(otable_contains(otable, ascii + 'i', 'i'));

    test_assert(otable_remove(otable, ascii + 'f', 'f'));
    otable_shrink(otable);
    test_assert(8 == otable_count(otable));
    otable_reserve_bits(otable, 8); // test resizing to 256 buckets

    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(otable_contains(otable, ascii + 'b', 'b'));
    test_assert(otable_contains(otable, ascii + 'c', 'c'));
    test_assert(otable_contains(otable, ascii + 'd', 'd'));
    test_assert(otable_contains(otable, ascii + 'e', 'e'));
    test_assert(!otable_contains(otable, ascii + 'f', 'f'));
    test_assert(otable_contains(otable, ascii + 'g', 'g'));
    test_assert(otable_contains(otable, ascii + 'h', 'h'));
    test_assert(otable_contains(otable, ascii + 'i', 'i'));

    test_assert(otable_remove(otable, ascii + 'b', 'b'));
    test_assert(otable_remove(otable, ascii + 'e', 'e'));
    test_assert(otable_remove(otable, ascii + 'i', 'i'));
    otable_shrink(otable);
    test_assert(5 == otable_count(otable));

    test_assert(otable_contains(otable, ascii + 'a', 'a'));
    test_assert(!otable_contains(otable, ascii + 'b', 'b'));
    test_assert(otable_contains(otable, ascii + 'c', 'c'));
    test_assert(otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));
    test_assert(!otable_contains(otable, ascii + 'f', 'f'));
    test_assert(otable_contains(otable, ascii + 'g', 'g'));
    test_assert(otable_contains(otable, ascii + 'h', 'h'));
    test_assert(!otable_contains(otable, ascii + 'i', 'i'));

    test_assert(otable_remove(otable, ascii + 'a', 'a'));
    test_assert(otable_remove(otable, ascii + 'h', 'h'));
    test_assert(otable_remove(otable, ascii + 'g', 'g'));
    test_assert(otable_remove(otable, ascii + 'c', 'c'));
    test_assert(otable_remove(otable, ascii + 'd', 'd'));
    otable_shrink(otable);
    test_assert(0 == otable_count(otable));

    test_assert(!otable_contains(otable, ascii + 'a', 'a'));
    test_assert(!otable_contains(otable, ascii + 'b', 'b'));
    test_assert(!otable_contains(otable, ascii + 'c', 'c'));
    test_assert(!otable_contains(otable, ascii + 'd', 'd'));
    test_assert(!otable_contains(otable, ascii + 'e', 'e'));
    test_assert(!otable_contains(otable, ascii + 'f', 'f'));
    test_assert(!otable_contains(otable, ascii + 'g', 'g'));
    test_assert(!otable_contains(otable, ascii + 'h', 'h'));
    test_assert(!otable_contains(otable, ascii + 'i', 'i'));

    otable_delete(otable);
}

static void test_remove_all_collision() {
    otable_t* otable = otable_new();

    // use hash 0 for all. should still work, degenerates into an array set

    otable_put(otable, ascii + 'a', 0);
    otable_put(otable, ascii + 'b', 0);
    otable_put(otable, ascii + 'c', 0);
    otable_put(otable, ascii + 'd', 0);
    otable_put(otable, ascii + 'e', 0);
    otable_put(otable, ascii + 'f', 0);
    otable_put(otable, ascii + 'g', 0);
    otable_put(otable, ascii + 'h', 0);
    otable_put(otable, ascii + 'i', 0);

    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(otable_contains(otable, ascii + 'b', 0));
    test_assert(otable_contains(otable, ascii + 'c', 0));
    test_assert(otable_contains(otable, ascii + 'd', 0));
    test_assert(otable_contains(otable, ascii + 'e', 0));
    test_assert(otable_contains(otable, ascii + 'f', 0));
    test_assert(otable_contains(otable, ascii + 'g', 0));
    test_assert(otable_contains(otable, ascii + 'h', 0));
    test_assert(otable_contains(otable, ascii + 'i', 0));

    test_assert(otable_remove(otable, ascii + 'f', 0));

    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(otable_contains(otable, ascii + 'b', 0));
    test_assert(otable_contains(otable, ascii + 'c', 0));
    test_assert(otable_contains(otable, ascii + 'd', 0));
    test_assert(otable_contains(otable, ascii + 'e', 0));
    test_assert(!otable_contains(otable, ascii + 'f', 0));
    test_assert(otable_contains(otable, ascii + 'g', 0));
    test_assert(otable_contains(otable, ascii + 'h', 0));
    test_assert(otable_contains(otable, ascii + 'i', 0));

    test_assert(otable_remove(otable, ascii + 'b', 0));
    test_assert(otable_remove(otable, ascii + 'e', 0));
    test_assert(otable_remove(otable, ascii + 'i', 0));

    test_assert(otable_contains(otable, ascii + 'a', 0));
    test_assert(!otable_contains(otable, ascii + 'b', 0));
    test_assert(otable_contains(otable, ascii + 'c', 0));
    test_assert(otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));
    test_assert(!otable_contains(otable, ascii + 'f', 0));
    test_assert(otable_contains(otable, ascii + 'g', 0));
    test_assert(otable_contains(otable, ascii + 'h', 0));
    test_assert(!otable_contains(otable, ascii + 'i', 0));

    test_assert(otable_remove(otable, ascii + 'a', 0));
    test_assert(otable_remove(otable, ascii + 'h', 0));
    test_assert(otable_remove(otable, ascii + 'g', 0));
    test_assert(otable_remove(otable, ascii + 'c', 0));
    test_assert(otable_remove(otable, ascii + 'd', 0));

    test_assert(!otable_contains(otable, ascii + 'a', 0));
    test_assert(!otable_contains(otable, ascii + 'b', 0));
    test_assert(!otable_contains(otable, ascii + 'c', 0));
    test_assert(!otable_contains(otable, ascii + 'd', 0));
    test_assert(!otable_contains(otable, ascii + 'e', 0));
    test_assert(!otable_contains(otable, ascii + 'f', 0));
    test_assert(!otable_contains(otable, ascii + 'g', 0));
    test_assert(!otable_contains(otable, ascii + 'h', 0));
    test_assert(!otable_contains(otable, ascii + 'i', 0));

    otable_delete(otable);
}

static void test_remove_many_collisions() {
    otable_t* otable = otable_new();

    // hash is low bit of letter. will cause many collisions.

    otable_put(otable, ascii + 'a', 'a' & 1);
    otable_put(otable, ascii + 'b', 'b' & 1);
    otable_put(otable, ascii + 'c', 'c' & 1);
    otable_put(otable, ascii + 'd', 'd' & 1);
    otable_put(otable, ascii + 'e', 'e' & 1);
    otable_put(otable, ascii + 'f', 'f' & 1);
    otable_put(otable, ascii + 'g', 'g' & 1);
    otable_put(otable, ascii + 'h', 'h' & 1);
    otable_put(otable, ascii + 'i', 'i' & 1);

    test_assert(otable_contains(otable, ascii + 'a', 'a' & 1));
    test_assert(otable_contains(otable, ascii + 'b', 'b' & 1));
    test_assert(otable_contains(otable, ascii + 'c', 'c' & 1));
    test_assert(otable_contains(otable, ascii + 'd', 'd' & 1));
    test_assert(otable_contains(otable, ascii + 'e', 'e' & 1));
    test_assert(otable_contains(otable, ascii + 'f', 'f' & 1));
    test_assert(otable_contains(otable, ascii + 'g', 'g' & 1));
    test_assert(otable_contains(otable, ascii + 'h', 'h' & 1));
    test_assert(otable_contains(otable, ascii + 'i', 'i' & 1));

    test_assert(otable_remove(otable, ascii + 'f', 'f' & 1));

    test_assert(otable_contains(otable, ascii + 'a', 'a' & 1));
    test_assert(otable_contains(otable, ascii + 'b', 'b' & 1));
    test_assert(otable_contains(otable, ascii + 'c', 'c' & 1));
    test_assert(otable_contains(otable, ascii + 'd', 'd' & 1));
    test_assert(otable_contains(otable, ascii + 'e', 'e' & 1));
    test_assert(!otable_contains(otable, ascii + 'f', 'f' & 1));
    test_assert(otable_contains(otable, ascii + 'g', 'g' & 1));
    test_assert(otable_contains(otable, ascii + 'h', 'h' & 1));
    test_assert(otable_contains(otable, ascii + 'i', 'i' & 1));

    test_assert(otable_remove(otable, ascii + 'b', 'b' & 1));
    test_assert(otable_remove(otable, ascii + 'e', 'e' & 1));
    test_assert(otable_remove(otable, ascii + 'i', 'i' & 1));

    test_assert(otable_contains(otable, ascii + 'a', 'a' & 1));
    test_assert(!otable_contains(otable, ascii + 'b', 'b' & 1));
    test_assert(otable_contains(otable, ascii + 'c', 'c' & 1));
    test_assert(otable_contains(otable, ascii + 'd', 'd' & 1));
    test_assert(!otable_contains(otable, ascii + 'e', 'e' & 1));
    test_assert(!otable_contains(otable, ascii + 'f', 'f' & 1));
    test_assert(otable_contains(otable, ascii + 'g', 'g' & 1));
    test_assert(otable_contains(otable, ascii + 'h', 'h' & 1));
    test_assert(!otable_contains(otable, ascii + 'i', 'i' & 1));

    test_assert(otable_remove(otable, ascii + 'a', 'a' & 1));
    test_assert(otable_remove(otable, ascii + 'h', 'h' & 1));
    test_assert(otable_remove(otable, ascii + 'g', 'g' & 1));
    test_assert(otable_remove(otable, ascii + 'c', 'c' & 1));
    test_assert(otable_remove(otable, ascii + 'd', 'd' & 1));

    test_assert(!otable_contains(otable, ascii + 'a', 'a' & 1));
    test_assert(!otable_contains(otable, ascii + 'b', 'b' & 1));
    test_assert(!otable_contains(otable, ascii + 'c', 'c' & 1));
    test_assert(!otable_contains(otable, ascii + 'd', 'd' & 1));
    test_assert(!otable_contains(otable, ascii + 'e', 'e' & 1));
    test_assert(!otable_contains(otable, ascii + 'f', 'f' & 1));
    test_assert(!otable_contains(otable, ascii + 'g', 'g' & 1));
    test_assert(!otable_contains(otable, ascii + 'h', 'h' & 1));
    test_assert(!otable_contains(otable, ascii + 'i', 'i' & 1));

    otable_delete(otable);
}

static bool contains(otable_t* otable, int letter, int hash) {
    for (void** entry = otable_find(otable, hash); entry;
            entry = otable_next(otable, entry, hash))
    {
        char* p = *entry;
        if (*p == letter) {
            return true;
        }
    }
    return false;
}

static void test_find() {
    otable_t* otable = otable_new();

    otable_put(otable, ascii + 'a', 'a');
    otable_put(otable, ascii + 'b', 'b');
    otable_put(otable, ascii + 'c', 'c');
    otable_put(otable, ascii + 'd', 'd');
    otable_put(otable, ascii + 'e', 'e');
    otable_put(otable, ascii + 'f', 'f');
    otable_put(otable, ascii + 'g', 'g');
    otable_put(otable, ascii + 'h', 'h');
    otable_put(otable, ascii + 'i', 'i');

    test_assert(contains(otable, 'a', 'a'));
    test_assert(contains(otable, 'b', 'b'));
    test_assert(contains(otable, 'c', 'c'));
    test_assert(contains(otable, 'd', 'd'));
    test_assert(contains(otable, 'e', 'e'));
    test_assert(contains(otable, 'f', 'f'));
    test_assert(contains(otable, 'g', 'g'));
    test_assert(contains(otable, 'h', 'h'));
    test_assert(contains(otable, 'i', 'i'));
    test_assert(!contains(otable, 'j', 'j'));
    test_assert(!contains(otable, 'k', 'k'));
    test_assert(!contains(otable, 'l', 'l'));
    test_assert(!contains(otable, 'm', 'm'));
    test_assert(!contains(otable, 'n', 'n'));
    test_assert(!contains(otable, 'o', 'o'));

    otable_delete(otable);
}

static void test_find_all_collision() {
    otable_t* otable = otable_new();

    otable_put(otable, ascii + 'a', 0);
    otable_put(otable, ascii + 'b', 0);
    otable_put(otable, ascii + 'c', 0);
    otable_put(otable, ascii + 'd', 0);
    otable_put(otable, ascii + 'e', 0);
    otable_put(otable, ascii + 'f', 0);
    otable_put(otable, ascii + 'g', 0);
    otable_put(otable, ascii + 'h', 0);
    otable_put(otable, ascii + 'i', 0);

    test_assert(contains(otable, 'a', 0));
    test_assert(contains(otable, 'b', 0));
    test_assert(contains(otable, 'c', 0));
    test_assert(contains(otable, 'd', 0));
    test_assert(contains(otable, 'e', 0));
    test_assert(contains(otable, 'f', 0));
    test_assert(contains(otable, 'g', 0));
    test_assert(contains(otable, 'h', 0));
    test_assert(contains(otable, 'i', 0));
    test_assert(!contains(otable, 'j', 0));
    test_assert(!contains(otable, 'k', 0));
    test_assert(!contains(otable, 'l', 0));
    test_assert(!contains(otable, 'm', 0));
    test_assert(!contains(otable, 'n', 0));
    test_assert(!contains(otable, 'o', 0));

    otable_delete(otable);
}

static void test_find_many_collisions() {
    otable_t* otable = otable_new();

    otable_put(otable, ascii + 'a', 'a' & 1);
    otable_put(otable, ascii + 'b', 'b' & 1);
    otable_put(otable, ascii + 'c', 'c' & 1);
    otable_put(otable, ascii + 'd', 'd' & 1);
    otable_put(otable, ascii + 'e', 'e' & 1);
    otable_put(otable, ascii + 'f', 'f' & 1);
    otable_put(otable, ascii + 'g', 'g' & 1);
    otable_put(otable, ascii + 'h', 'h' & 1);
    otable_put(otable, ascii + 'i', 'i' & 1);

    test_assert(contains(otable, 'a', 'a' & 1));
    test_assert(contains(otable, 'b', 'b' & 1));
    test_assert(contains(otable, 'c', 'c' & 1));
    test_assert(contains(otable, 'd', 'd' & 1));
    test_assert(contains(otable, 'e', 'e' & 1));
    test_assert(contains(otable, 'f', 'f' & 1));
    test_assert(contains(otable, 'g', 'g' & 1));
    test_assert(contains(otable, 'h', 'h' & 1));
    test_assert(contains(otable, 'i', 'i' & 1));
    test_assert(!contains(otable, 'j', 'j' & 1));
    test_assert(!contains(otable, 'k', 'k' & 1));
    test_assert(!contains(otable, 'l', 'l' & 1));
    test_assert(!contains(otable, 'm', 'm' & 1));
    test_assert(!contains(otable, 'n', 'n' & 1));
    test_assert(!contains(otable, 'o', 'o' & 1));

    otable_delete(otable);
}

static void test_find_random() {
    otable_t* otable = otable_new();

    // test that reserve works
    otable_reserve_bits(otable, 6);

    bool exists[128] = {};
    size_t count = 0;
    srand(0);

    // randomly insert or remove characters for a while.
    for (size_t i = 0; i < 10000; ++i) {
        char c = 32 + rand() % 95; // printable only for easy debugging
        //printf("\n  char %i %c %s\n", c, c, exists[c]?"exists":"does not exist");

        test_assert(exists[c] == otable_contains(otable, ascii + c,  c));
        if (exists[c]) {
            --count;
            test_assert(otable_remove(otable, ascii + c, c));
        } else {
            ++count;
            test_assert(otable_put(otable, ascii + c, c));
        }
        exists[c] = !exists[c];
        test_assert(count == otable_count(otable));
    }

    otable_delete(otable);
}

static void test_union() {
    otable_t* t1 = otable_new();
    otable_t* t2 = otable_new();

    otable_put(t1, ascii + 'a', 'a');
    otable_put(t1, ascii + 'b', 'b');
    otable_put(t1, ascii + 'c', 'c');
    otable_put(t1, ascii + 'd', 'd');

    otable_put(t2, ascii + 'e', 'e');
    otable_put(t2, ascii + 'f', 'f');
    otable_put(t2, ascii + 'g', 'g');
    otable_put(t2, ascii + 'h', 'h');
    otable_put(t2, ascii + 'i', 'i');

    test_assert(contains(t1, 'a', 'a'));
    test_assert(contains(t1, 'b', 'b'));
    test_assert(contains(t1, 'c', 'c'));
    test_assert(contains(t1, 'd', 'd'));
    test_assert(!contains(t1, 'e', 'e'));
    test_assert(!contains(t1, 'f', 'f'));
    test_assert(!contains(t1, 'g', 'g'));
    test_assert(!contains(t1, 'h', 'h'));
    test_assert(!contains(t1, 'i', 'i'));

    test_assert(!contains(t2, 'a', 'a'));
    test_assert(!contains(t2, 'b', 'b'));
    test_assert(!contains(t2, 'c', 'c'));
    test_assert(!contains(t2, 'd', 'd'));
    test_assert(contains(t2, 'e', 'e'));
    test_assert(contains(t2, 'f', 'f'));
    test_assert(contains(t2, 'g', 'g'));
    test_assert(contains(t2, 'h', 'h'));
    test_assert(contains(t2, 'i', 'i'));

    otable_union(t1, t2);

    test_assert(contains(t1, 'a', 'a'));
    test_assert(contains(t1, 'b', 'b'));
    test_assert(contains(t1, 'c', 'c'));
    test_assert(contains(t1, 'd', 'd'));
    test_assert(contains(t1, 'e', 'e'));
    test_assert(contains(t1, 'f', 'f'));
    test_assert(contains(t1, 'g', 'g'));
    test_assert(contains(t1, 'h', 'h'));
    test_assert(contains(t1, 'i', 'i'));

    test_assert(!contains(t2, 'a', 'a'));
    test_assert(!contains(t2, 'b', 'b'));
    test_assert(!contains(t2, 'c', 'c'));
    test_assert(!contains(t2, 'd', 'd'));
    test_assert(contains(t2, 'e', 'e'));
    test_assert(contains(t2, 'f', 'f'));
    test_assert(contains(t2, 'g', 'g'));
    test_assert(contains(t2, 'h', 'h'));
    test_assert(contains(t2, 'i', 'i'));

    otable_union(t2, t1);

    test_assert(contains(t1, 'a', 'a'));
    test_assert(contains(t1, 'b', 'b'));
    test_assert(contains(t1, 'c', 'c'));
    test_assert(contains(t1, 'd', 'd'));
    test_assert(contains(t1, 'e', 'e'));
    test_assert(contains(t1, 'f', 'f'));
    test_assert(contains(t1, 'g', 'g'));
    test_assert(contains(t1, 'h', 'h'));
    test_assert(contains(t1, 'i', 'i'));

    test_assert(contains(t2, 'a', 'a'));
    test_assert(contains(t2, 'b', 'b'));
    test_assert(contains(t2, 'c', 'c'));
    test_assert(contains(t2, 'd', 'd'));
    test_assert(contains(t2, 'e', 'e'));
    test_assert(contains(t2, 'f', 'f'));
    test_assert(contains(t2, 'g', 'g'));
    test_assert(contains(t2, 'h', 'h'));
    test_assert(contains(t2, 'i', 'i'));

    otable_delete(t2);
    otable_delete(t1);
}

static void test_union_all_collision() {
    otable_t* t1 = otable_new();
    otable_t* t2 = otable_new();

    otable_put(t1, ascii + 'a', 0);
    otable_put(t1, ascii + 'b', 0);
    otable_put(t1, ascii + 'c', 0);
    otable_put(t1, ascii + 'd', 0);

    otable_put(t2, ascii + 'e', 0);
    otable_put(t2, ascii + 'f', 0);
    otable_put(t2, ascii + 'g', 0);
    otable_put(t2, ascii + 'h', 0);
    otable_put(t2, ascii + 'i', 0);

    test_assert(contains(t1, 'a', 0));
    test_assert(contains(t1, 'b', 0));
    test_assert(contains(t1, 'c', 0));
    test_assert(contains(t1, 'd', 0));
    test_assert(!contains(t1, 'e', 0));
    test_assert(!contains(t1, 'f', 0));
    test_assert(!contains(t1, 'g', 0));
    test_assert(!contains(t1, 'h', 0));
    test_assert(!contains(t1, 'i', 0));

    test_assert(!contains(t2, 'a', 0));
    test_assert(!contains(t2, 'b', 0));
    test_assert(!contains(t2, 'c', 0));
    test_assert(!contains(t2, 'd', 0));
    test_assert(contains(t2, 'e', 0));
    test_assert(contains(t2, 'f', 0));
    test_assert(contains(t2, 'g', 0));
    test_assert(contains(t2, 'h', 0));
    test_assert(contains(t2, 'i', 0));

    otable_union(t1, t2);

    test_assert(contains(t1, 'a', 0));
    test_assert(contains(t1, 'b', 0));
    test_assert(contains(t1, 'c', 0));
    test_assert(contains(t1, 'd', 0));
    test_assert(contains(t1, 'e', 0));
    test_assert(contains(t1, 'f', 0));
    test_assert(contains(t1, 'g', 0));
    test_assert(contains(t1, 'h', 0));
    test_assert(contains(t1, 'i', 0));

    test_assert(!contains(t2, 'a', 0));
    test_assert(!contains(t2, 'b', 0));
    test_assert(!contains(t2, 'c', 0));
    test_assert(!contains(t2, 'd', 0));
    test_assert(contains(t2, 'e', 0));
    test_assert(contains(t2, 'f', 0));
    test_assert(contains(t2, 'g', 0));
    test_assert(contains(t2, 'h', 0));
    test_assert(contains(t2, 'i', 0));

    otable_union(t2, t1);

    test_assert(contains(t1, 'a', 0));
    test_assert(contains(t1, 'b', 0));
    test_assert(contains(t1, 'c', 0));
    test_assert(contains(t1, 'd', 0));
    test_assert(contains(t1, 'e', 0));
    test_assert(contains(t1, 'f', 0));
    test_assert(contains(t1, 'g', 0));
    test_assert(contains(t1, 'h', 0));
    test_assert(contains(t1, 'i', 0));

    test_assert(contains(t2, 'a', 0));
    test_assert(contains(t2, 'b', 0));
    test_assert(contains(t2, 'c', 0));
    test_assert(contains(t2, 'd', 0));
    test_assert(contains(t2, 'e', 0));
    test_assert(contains(t2, 'f', 0));
    test_assert(contains(t2, 'g', 0));
    test_assert(contains(t2, 'h', 0));
    test_assert(contains(t2, 'i', 0));

    otable_delete(t2);
    otable_delete(t1);
}

static void test_union_many_collisions() {
    otable_t* t1 = otable_new();
    otable_t* t2 = otable_new();

    otable_put(t1, ascii + 'a', 'a' & 1);
    otable_put(t1, ascii + 'b', 'b' & 1);
    otable_put(t1, ascii + 'c', 'c' & 1);
    otable_put(t1, ascii + 'd', 'd' & 1);

    otable_put(t2, ascii + 'e', 'e' & 1);
    otable_put(t2, ascii + 'f', 'f' & 1);
    otable_put(t2, ascii + 'g', 'g' & 1);
    otable_put(t2, ascii + 'h', 'h' & 1);
    otable_put(t2, ascii + 'i', 'i' & 1);

    test_assert(contains(t1, 'a', 'a' & 1));
    test_assert(contains(t1, 'b', 'b' & 1));
    test_assert(contains(t1, 'c', 'c' & 1));
    test_assert(contains(t1, 'd', 'd' & 1));
    test_assert(!contains(t1, 'e', 'e' & 1));
    test_assert(!contains(t1, 'f', 'f' & 1));
    test_assert(!contains(t1, 'g', 'g' & 1));
    test_assert(!contains(t1, 'h', 'h' & 1));
    test_assert(!contains(t1, 'i', 'i' & 1));

    test_assert(!contains(t2, 'a', 'a' & 1));
    test_assert(!contains(t2, 'b', 'b' & 1));
    test_assert(!contains(t2, 'c', 'c' & 1));
    test_assert(!contains(t2, 'd', 'd' & 1));
    test_assert(contains(t2, 'e', 'e' & 1));
    test_assert(contains(t2, 'f', 'f' & 1));
    test_assert(contains(t2, 'g', 'g' & 1));
    test_assert(contains(t2, 'h', 'h' & 1));
    test_assert(contains(t2, 'i', 'i' & 1));

    otable_union(t1, t2);

    test_assert(contains(t1, 'a', 'a' & 1));
    test_assert(contains(t1, 'b', 'b' & 1));
    test_assert(contains(t1, 'c', 'c' & 1));
    test_assert(contains(t1, 'd', 'd' & 1));
    test_assert(contains(t1, 'e', 'e' & 1));
    test_assert(contains(t1, 'f', 'f' & 1));
    test_assert(contains(t1, 'g', 'g' & 1));
    test_assert(contains(t1, 'h', 'h' & 1));
    test_assert(contains(t1, 'i', 'i' & 1));

    test_assert(!contains(t2, 'a', 'a' & 1));
    test_assert(!contains(t2, 'b', 'b' & 1));
    test_assert(!contains(t2, 'c', 'c' & 1));
    test_assert(!contains(t2, 'd', 'd' & 1));
    test_assert(contains(t2, 'e', 'e' & 1));
    test_assert(contains(t2, 'f', 'f' & 1));
    test_assert(contains(t2, 'g', 'g' & 1));
    test_assert(contains(t2, 'h', 'h' & 1));
    test_assert(contains(t2, 'i', 'i' & 1));

    otable_union(t2, t1);

    test_assert(contains(t1, 'a', 'a' & 1));
    test_assert(contains(t1, 'b', 'b' & 1));
    test_assert(contains(t1, 'c', 'c' & 1));
    test_assert(contains(t1, 'd', 'd' & 1));
    test_assert(contains(t1, 'e', 'e' & 1));
    test_assert(contains(t1, 'f', 'f' & 1));
    test_assert(contains(t1, 'g', 'g' & 1));
    test_assert(contains(t1, 'h', 'h' & 1));
    test_assert(contains(t1, 'i', 'i' & 1));

    test_assert(contains(t2, 'a', 'a' & 1));
    test_assert(contains(t2, 'b', 'b' & 1));
    test_assert(contains(t2, 'c', 'c' & 1));
    test_assert(contains(t2, 'd', 'd' & 1));
    test_assert(contains(t2, 'e', 'e' & 1));
    test_assert(contains(t2, 'f', 'f' & 1));
    test_assert(contains(t2, 'g', 'g' & 1));
    test_assert(contains(t2, 'h', 'h' & 1));
    test_assert(contains(t2, 'i', 'i' & 1));

    otable_delete(t2);
    otable_delete(t1);
}

int main(void) {
    init_ascii();
    test_simple();
    test_simple_all_collision();
    test_remove();
    test_remove_all_collision();
    test_remove_many_collisions();
    test_find();
    test_find_all_collision();
    test_find_many_collisions();
    test_find_random();
    test_union();
    test_union_all_collision();
    test_union_many_collisions();
}
