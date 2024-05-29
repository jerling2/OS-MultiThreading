#ifndef THREAD_H
#define THREAD_H

typedef struct thread_arg {
    int myid;
    int total;
} thread_arg;

void obtain_locks(int num, ...);

void release_locks(int num, ...);

int increasing_order(const void *a, const void *b);

int decreasing_order(const void *a, const void *b);

#endif