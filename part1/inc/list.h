#ifndef LIST_H
#define LIST_H

typedef struct hashmap {
    unsigned int size;
    struct list **map;
} hashmap;

typedef struct list {
    unsigned int size;
    struct node *head;
    struct node *tail;
} list;

typedef struct node {
    void *data;
    struct node *next;
    struct node *prev;
} node;

typedef void (*freefun)(void *);

list *initList();
node *initnode();
int enqueue(list *l, void *data);
void *dequeue(list *l);
void *inorder(list *l, node **cnode);
void freeList(list *l, freefun release);

// Dan Bernstein's DJB2 Hash Algorithm (http://www.cse.yorku.ca/~oz/hash.html)
unsigned long hash(unsigned char *str);

hashmap *initHashmap(unsigned int size);
void freeHashmap(hashmap *hm, freefun freemem);
void insert(hashmap *hm, char *key, void *data);

unsigned int nextPowerOf2(unsigned int number);

#endif /* QUEUE_H */