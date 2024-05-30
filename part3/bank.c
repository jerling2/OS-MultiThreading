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
requestCounter *rc;
char *filename;
hashmap *account_hashmap;
account **account_array;
int numacs;

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

    // Spagetti code
    rc = (requestCounter *)malloc(sizeof(requestCounter));
    rc->count = 0;
    pthread_mutex_init(&(rc->rc_lock), NULL);
    // end

    getAccounts(stream, filename, &account_hashmap, &account_array, &numacs);
    if (numacs == 0) {                              // Handle getAccount Error.
        fclose(stream);
        exit(EXIT_FAILURE);   
    }

    for (i=0; i<10; i++) {
        thread_arg *arg = (thread_arg *)malloc(sizeof(thread_arg));
        arg->myid = i;
        arg->total = 9;
        pthread_create(&worker_threads[i], NULL, process_transaction, (void *)arg);
    }

    for (i=0; i<10; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    // CREATE BANKER THREAD
    pthread_create(&banker_thread, NULL, update_balance, NULL);
    pthread_join(banker_thread, NULL);

    // MAIN THREAD STUFF
    print_balances(account_array, numacs);
    freeHashmap(account_hashmap, (void *)freeacc);
    free(account_array);
    printf("rc->count=%d\n",rc->count);
    free(rc);
    fclose(stream);
}

// GLOBAL : hashmap (read only), numacs (read only), filename (read only)
// STACK : Line number , request, stream*

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

    while ((request = readRequest(stream_copy)) != NULL) {
        if (commandInterpreter(account_hashmap, request, rc) == -1)
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
    process_reward(account_array, numacs);
}