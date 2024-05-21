#ifndef LIST_H
#define LIST_H

typedef struct list {
    int size;
    struct node *head;
    struct node *tail;
} list;

typedef struct node {
    void *data;
    struct node *next;
    struct node *prev;
} node;

typedef void (*freefun)(void *);

list *initlist();
node *initnode();
int enqueue(list *l, void *data);
void *dequeue(list *l);
void *inorder(list *l, node **cnode);
void freelist(list *l, freefun release);

#endif /* QUEUE_H */