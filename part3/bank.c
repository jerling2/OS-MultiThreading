#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "fileio.h"
#include "list.h"
#include "parser.h"
#include "request.h"
#include "thread.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define WARNING "\x1b[1;31mWARNING\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"
#define REQUEST "%s invalid request %s:%d.\n"


void *process_transaction (void *arg);
void *update_balance (void *arg);
char *filename;
hashmap *account_hashmap;
account **account_array;
int numacs;
int bank_running;
pthread_barrier_t worker_start_barrier;


threadMediator worker_bank_sync = {
    .sync_lock = PTHREAD_MUTEX_INITIALIZER,
    .cond1 = PTHREAD_COND_INITIALIZER,
    .cond2 = PTHREAD_COND_INITIALIZER
};


requestCounter rc = {
    .rc_lock = PTHREAD_MUTEX_INITIALIZER,
    .count = 0,
};


int main (int argc, char *argv[]) 
{
    FILE *stream;
    pthread_t worker_threads[10];
    pthread_t banker_thread;
    int i;

    if (argc != 2) {                                         // Validate Input.
        fprintf(stderr, USUAGE, ERROR, argv[0]);
        exit(EXIT_FAILURE); 
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {           // Test the stream.
        printf(STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    getAccounts(stream, filename, &account_hashmap, &account_array, &numacs);
    if (numacs == 0) {                              // Handle getAccount Error.
        fclose(stream);
        exit(EXIT_FAILURE);   
    }

    // Create bank thread.
    bank_running = 1;
    pthread_mutex_lock(&worker_bank_sync.sync_lock);
    pthread_create(&banker_thread, NULL, update_balance, NULL);
    pthread_cond_wait(&worker_bank_sync.cond2, &worker_bank_sync.sync_lock);
    pthread_mutex_unlock(&worker_bank_sync.sync_lock);

    // Create Barrier for 11 threads (10 workers + 1 main)
    pthread_barrier_init(&worker_start_barrier, NULL, 11); 

    // Create worker threads.
    for (i=0; i<10; i++) {
        thread_arg *arg = (thread_arg *)malloc(sizeof(thread_arg));
        arg->myid = i;
        arg->total = 9;
        pthread_create(&worker_threads[i], NULL, process_transaction, (void *)arg);
    }

    // Wait until all worker threads are ready.
    pthread_barrier_wait(&worker_start_barrier);

    // Wait for workers to exit.
    for (i=0; i<10; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    
    // Terminate the bank thread.
    bank_running = 0;
    pthread_cond_signal(&worker_bank_sync.cond1);
    pthread_join(banker_thread, NULL);

    print_balances(account_array, numacs);
    freeHashmap(account_hashmap, (void *)freeacc);
    free(account_array);
    fclose(stream);
}


// WORKER THREAD
void *process_transaction (void *arg)
{
    FILE *stream_copy;
    int line_number;
    cmd *request;
    int i;
    char line[BUFSIZ];
    int offset = ((thread_arg *)arg)->myid;
    int total = ((thread_arg *)arg)->total;
    
    free(arg);
    line_number = numacs*5+1;
    stream_copy = fopen(filename, "r");
    
    for (i = 0; i<line_number; i++)
        fgets(line, BUFSIZ, stream_copy); // skip the first 51 lines

    for (i = 0; i<offset; i++)
        fgets(line, BUFSIZ, stream_copy); // skip the next "offset" lines.


    // Barrier
    pthread_barrier_wait(&worker_start_barrier); // Need 11 threads waiting here 
    
    while ((request = readRequest(stream_copy)) != NULL) {
        if (commandInterpreter(account_hashmap, request, &rc) == -1)
            fprintf(stderr, REQUEST, WARNING, filename, line_number); // DEBUG.
        freecmd(request);
        for (i = 0; i<total; i++)
            fgets(line, BUFSIZ, stream_copy); // skip the next N lines.
        line_number += total+1;
    }
    fclose(stream_copy);
    return NULL;
}


// BANK THREAD
void *update_balance (void *arg)
{
    pthread_mutex_lock(&worker_bank_sync.sync_lock);
    pthread_cond_signal(&worker_bank_sync.cond2); // Signal main
    while (bank_running) {
        pthread_cond_wait(&worker_bank_sync.cond1, &worker_bank_sync.sync_lock);
        pthread_cond_signal(&worker_bank_sync.cond2);
        process_reward(account_array, numacs);
    }
}

// MAIN: A- *w || (C1, A+-)
// WORK:       || A- (if not last: (C2, A+-), else: C1, (C2, A+-))
// Barrier lets main know when all threads are created
// Barrier lets all threads start at some time