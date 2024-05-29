#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "thread.h"
#include "account.h"


void obtain_locks(int num, ...)
{
    account **array;
    va_list args;
    va_start(args, num);
    int i;

    array = (account **) malloc(sizeof(account *)*num);
    for (i = 0; i<num; i++) {
        array[i] = va_arg(args, account *);
    }
    qsort(array, num, sizeof(account *), increasing_order);
    for (i = 0; i<num; i++)
        pthread_mutex_lock(&array[i]->ac_lock);
    free(array);
    va_end(args);        
}


void release_locks(int num, ...)
{
    account **array;
    va_list args;
    va_start(args, num);
    int i;

    array = (account **) malloc(sizeof(account *)*num);
    for (i = 0; i<num; i++) {
        array[i] = va_arg(args, account *);
    }
    qsort(array, num, sizeof(account *), decreasing_order);
    for (i = 0; i<num; i++)
        pthread_mutex_unlock(&array[i]->ac_lock);
    free(array);
    va_end(args);   
}


int increasing_order(const void *a, const void *b) {
    return ((*(account **)a)->order - (*(account **)b)->order);
}

int decreasing_order(const void *a, const void *b) {
    return ((*(account **)b)->order - (*(account **)a)->order);
}