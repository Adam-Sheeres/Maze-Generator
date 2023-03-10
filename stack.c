#include "stack.h"

bool isEmpty(struct stack *root) {
    return !root;
}

struct stack *newNode(struct coords *data) {
    struct stack *node = (struct stack*)malloc(sizeof(struct stack));
    struct coords *newCoords = (struct coords*)malloc(sizeof(struct coords));
    newCoords->x = data->x;
    newCoords->y = data->y;
    
    node->data = newCoords;
    node->next = NULL;
    return node;
}

void push(struct stack **root, struct coords*data) {
    struct stack *newStack = newNode(data);
    newStack->next = *root;
    *root = newStack;
}

struct coords *pop(struct stack **root) {
    if (isEmpty(*root)) return NULL;

    struct stack* temp = *root;

    *root = (*root)->next;
    struct coords *d = temp->data;
    free(temp);

    return d;
}

struct coords *peek(struct stack **root) {
    struct stack *temp = *root;
    struct coords *d = temp->data;
    return d;
}