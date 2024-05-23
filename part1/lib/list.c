#include <stdlib.h>
#include "list.h"


list *initList()
{
    list *newlist;
    newlist = (list *)malloc(sizeof(list));
    newlist->size = 0;
    newlist->head = NULL;
    newlist->tail = NULL;
    return newlist;
}


node *initnode()
{
    node *newnode;
    newnode = (node *)malloc(sizeof(node));
    newnode->data = NULL;
    newnode->next = NULL;
    newnode->prev = NULL;
    return newnode;
}


void *inorder(list *l, node **cnode)
{ 
    if (l->size == 0) {
        return NULL;
    }
    if (*cnode == NULL) {
        *cnode = l->head;
        return (*cnode)->data;
    }
    *cnode = (*cnode)->next;
    if (*cnode == NULL) {
        return NULL;
    }
    return (*cnode)->data;
}


int enqueue(list *l, void *data)
{
    node *newnode;
    node *cnode;

    newnode = initnode();
    newnode->data = data;
    if (l->size == 0) {
        l->head = newnode;
        l->tail = newnode;
        l->size ++;
        return 0;
    }
    cnode = l->head;
    while (cnode->next != NULL) {
        if (cnode->data == data) {
            free(newnode);
            return -1;
        }
        cnode = cnode->next;
    }
    cnode->next = newnode;
    newnode->prev = cnode;
    l->tail = newnode;
    l->size++;
    return 0;
}


void *dequeue(list *l)
{
    void *data;
    node *removed;

    if (l->size == 0)
        return NULL;
    data = l->head->data;
    removed = l->head;
    if (l->size == 1) {                      
        l->head = NULL;
        l->tail = NULL;
    } else {
        l->head = l->head->next;
    }
    free(removed);
    l->size--;
    return data;
}


void freeList(list *l, freefun freemem)
{
    void *data;
    
    if (l == NULL) {
        return;
    }
    while ((data = dequeue(l)) != NULL) {
        freemem(data); 
    }
    free(l);
}


// Dan Bernstein's DJB2 Hash Algorithm (http://www.cse.yorku.ca/~oz/hash.html)
unsigned long 
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


hashmap *initHashmap(int size)
{
    hashmap *hm;
    int i;
    
    hm = (hashmap *) malloc (sizeof(hashmap));
    hm->size = size;
    hm->map = (list **) malloc(sizeof(list *)*size);
    for (i = 0; i<size; i++) {
        hm->map[i] = NULL;
    }
    return hm;
}


void freeHashmap(hashmap *hm, freefun freemem)
{
    int i;

    for (i = 0; i<hm->size; i++) {
        if (hm->map[i] == NULL)
            freeList(hm->map[i], freemem);
    }
    free(hm);
}


void insert(hashmap *hm, unsigned char *key, void *data)
{
    unsigned long hashvalue;
    int index;

    hashvalue = hash(key);
    index = hashvalue % hm->size;
    if (hm->map[index] == NULL)
        hm->map[index] = initList();
    enqueue(hm->map[index], data);
}