#ifndef SIMPLE_HMAP_
#define SIMPLE_HMAP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_ll.h"

typedef struct hmap {
    int size;
    node **nodes;
} hmap;

void free_node(node *n);

void hmap_init(hmap *h, size_t size);

int hmap_insert(hmap *h, const char *key, const void *value, size_t payload_size);

int hmap_append_str(
        hmap *h,
        const char *key,
        const char *value,
        const char *divider);

void *hmap_lookup(hmap *h, const char *key);

int hmap_hash(hmap *h, const char *key);

void hmap_disp(hmap *h);

#endif