#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct __attribute__((__packed__)) node {
	struct node * next;
	char * key;
	size_t payload_size;
	void * value;
} node;

typedef struct hmap {
	int size;
	node ** nodes;
} hmap;

void hmap_init(hmap * h, size_t size){
	h->size = size;
	h->nodes = malloc(sizeof(node *) * size);
	memset(h->nodes, 0, sizeof(node *) * size);
}

node * hmap_init_node(const char * key, const void * value, size_t payload_size) {
	// prep the new node with the appropriate data
	node * new_node = malloc(sizeof(node));
	new_node->next = NULL;

	new_node->key = malloc(strlen(key) + 1);
	memcpy(new_node->key, key, strlen(key) + 1);
	
	new_node->payload_size = payload_size;
	if (payload_size > 0) {
		new_node->value = malloc(payload_size);
		memcpy(new_node->value, value, payload_size);
	} else{
		new_node->value = NULL;
	}

	return new_node;
}

int hmap_hash(hmap * h, const char * key) {
	int sum = 0;
	for (;*key != '\0'; key++) {
		sum += *key;
	}
	return sum % (h->size);
}

// inserts something into a hashmap
// if the hashmap already contains a value for that key, fails and returns 0
int hmap_insert(hmap * h, const char * key, const void * value, size_t payload_size) {
	int hash = hmap_hash(h,key);
	printf("'%s' hashes to %d, inserting\n", key, hash);
	node * head = h->nodes[hash];
	node * new_node = hmap_init_node(key, value, payload_size);

	if (h->nodes[hash] == NULL) {
		h->nodes[hash] = new_node;
		return 1;
	} else {

		while(head->next != NULL) {
			if (0 == strcmp(key, head->key)) {
				return 0;
			} else {
				head = head->next;
			}
		}

		head->next = new_node;
		return 1;
	}
	
}

void * hmap_lookup(hmap *h, const char * key) {
	int hash = hmap_hash(h,key);
	printf("'%s' hashes to %d, performing lookup\n", key, hash);
	node * head = h->nodes[hash];
	while(head != NULL) {
		if (0 == strcmp(key, head->key)) {
			return head->value;
		} else {
			head = head->next;
		}
	}
	return NULL;
}