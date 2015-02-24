#ifndef SIMPLE_LL_
#define SIMPLE_LL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) node {
	struct node * next;
	char * key;
	size_t payload_size;
	void * value;
} node;

node * init_node(const char * key, const void * value, size_t payload_size);

typedef struct ll {
	int size;
	node * head;
	node * tail;
} ll;

ll * ll_init();
void ll_print(ll * l);
void ll_append(ll *l, const char * value);
void ll_from_string(ll * l, const char* str, const char separator);
char * ll_pop_head(ll *l);
bool ll_contains(ll *l, char *str);

#endif