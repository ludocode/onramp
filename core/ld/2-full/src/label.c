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

#include "label.h"

#include "common.h"
#include "string.h"



/*
 * Label
 */

label_t* label_new(string_t* name) {
    label_t* label = malloc(sizeof(label_t));
    label->name = name;
    return label;
}

void label_delete(label_t* label) {
    string_deref(label->name);
    free(label);
}



/*
 * Label hashtable
 */

// label hashtable (in the current file)
static label_t** labels;
#define LABELS_SIZE 128
#define LABELS_MASK 127

void labels_init(void) {
    labels = calloc(LABELS_SIZE, sizeof(label_t*));
}

void labels_destroy(void) {
    labels_clear();
    free(labels);
}

label_t* labels_find(const char* bytes, size_t length) {
    uint32_t hash = fnv1a_bytes(bytes, length);
    label_t* label = labels[hash & LABELS_MASK];
    while (label) {
        if (string_equal_bytes(label->name, bytes, length)) {
            return label;
        }
        label = label->next;
    }
    return NULL;
}

void labels_insert(label_t* label) {
    size_t index = string_hash(label->name) & LABELS_MASK;
    label->next = labels[index];
    labels[index] = label;
}

void labels_clear(void) {
    for (size_t i = 0; i < LABELS_SIZE; ++i) {
        label_t* label = labels[i];
        while (label) {
            label_t* next = label->next;
            label_delete(label);
            label = next;
        }
        labels[i] = NULL;
    }
}

label_t* labels_define(const char* bytes, size_t length) {
    string_t* string = string_intern_bytes(bytes, length);
    label_t* label = label_new(string);
    labels_insert(label);
    return label;
}
