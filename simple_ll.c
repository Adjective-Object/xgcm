#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "simple_ll.h"

node * init_node(const char * key, const void * value, size_t payload_size) {
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

ll * ll_init(){
	ll * l = malloc(sizeof(ll));
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
	return l;
}

void ll_append(ll *l, const char * value) {
	node * next = init_node(value, NULL, 0);
	
	if (l->tail != NULL) {
		l->tail->next = next;
		l->tail = next;
	}
	else {
		l->tail = next;
		l->head = next;
	}

	l->size++;
}

char * ll_pop_head(ll *l) {
	if (l->head == NULL) {
		printf("error popping head -- THERE IS NO HEAD\n");
		return NULL;
	}
	
	node * oldhead = l->head;
	l->head = oldhead->next;

	char * oldhead_value = oldhead->key;
	free(oldhead);

	l->size --;
	if (l->head == NULL) {
		l->tail = NULL;
	}

	return oldhead_value;
}

void ll_from_string(ll * l, const char * sepstring, const char separator){

    char * buffer = malloc(strlen(sepstring) + 1);
    memcpy(buffer, sepstring, strlen(sepstring) + 1);
    char * head = buffer;
    char * tail = buffer;
    
    for(; *head != '\0'; head++) {
        if (*head == separator && tail < head) {
            *head = '\0';
          	ll_append(l , tail);
            tail = (char *)(head +1);
        }
    }
    ll_append(l, tail); 
    free(buffer);
}


void ll_print(ll * l) {
	node * head = l->head;
	printf("[");
	if(head !=NULL) {
		printf(" %s", head->key);
		head = head->next;
	}
	while (head != NULL) {
		printf(",\n  %s", head->key);
		head = head->next;
	}
	printf(" ]\n");
}


bool ll_contains(ll *l, char *str) {
	node * current = l->head;
	while (current != NULL) {
		if (0 == strcmp(current->key, str))
			return true;
		current = current->next;
	}
	return false;
}