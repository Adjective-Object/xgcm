#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_hmap.h"

void hmap_init(hmap *h, size_t size) {
    h->size = size;
    h->nodes = malloc(sizeof(node *) * size);
    memset(h->nodes, 0, sizeof(node *) * size);
}

void free_node(node *node) {
    free(node->value);
    free(node->key);
    free(node);
}

int hmap_hash(hmap *h, const char *key) {
    int sum = 0;
    for (; *key != '\0'; key++) {
        sum += *key;
    }
    return sum % (h->size);
}

// inserts something into a hashmap
// if the hashmap already contains a value for that key, fail and return 0
int hmap_insert(hmap *h, const char *key, const void *value, size_t payload_size) {
    int hash = hmap_hash(h, key);
    node *head = h->nodes[hash];
    node *new_node = init_node(key, value, payload_size);

    if (h->nodes[hash] == NULL) {
        h->nodes[hash] = new_node;
    } else {

        while (head != NULL) {
            if (0 == strcmp(key, head->key)) {
                return 0;
            } else {
                head = head->next;
            }
        }

        head->next = new_node;
    }
    return 1;
}

// inserts something into a hashmap
// if the hashmap already contains a value for that key, strcat it , appending 0
int hmap_append_str(
        hmap *h,
        const char *key,
        const char *value,
        const char *divider) {


    size_t vallen = strlen(value);
    int hash = hmap_hash(h, key);
    node *head = h->nodes[hash];
    node *new_node = init_node(key, value, vallen + 1);


    if (h->nodes[hash] == NULL) {
        h->nodes[hash] = new_node;
    }
    else {

        node *prev = head;
        while (head != NULL) {
            if (0 == strcmp(key, head->key)) {

                size_t divlen = strlen(divider);
                size_t new_size = head->payload_size + vallen + divlen;

                char *new_value = malloc(sizeof(char) * new_size);
                memcpy(
                        new_value,
                        head->value,
                        head->payload_size - 1);
                strcpy(
                        new_value + head->payload_size - 1,
                        divider);
                strcpy(
                        new_value + head->payload_size - 1 + divlen,
                        value);

                free(head->value);
                head->value = new_value;
                head->payload_size = new_size;
                return 0;
            } else {
                prev = head;
                head = head->next;
            }
        }
        prev->next = new_node;
    }
    return 1;
}

void *hmap_lookup(hmap *h, const char *key) {
    int hash = hmap_hash(h, key);
    node *head = h->nodes[hash];
    while (head != NULL) {
        if (0 == strcmp(key, head->key)) {
            return head->value;
        } else {
            head = head->next;
        }
    }
    return NULL;
}

void hmap_disp(hmap *h) {
    int i;
    for (i = 0; i < h->size; i++) {
        if (h->nodes[i] != NULL) {

            node *n = h->nodes[i];
            while (n != NULL) {
                printf("    %s: %.*s\n",
                        n->key,
                        (int) n->payload_size,
                        (char *) n->value);

                n = n->next;
            }
        }
    }
}