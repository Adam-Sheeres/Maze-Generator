#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct coords {
    int x;
    int y;
} coords;

typedef struct stack {
    struct coords *data;
    struct stack *next;
} stack;

bool isEmpty(struct stack *);
void push(struct stack **, struct coords*);
struct coords *pop(struct stack **);
struct stack *newNode(struct coords *);
struct coords *peek(struct stack **);