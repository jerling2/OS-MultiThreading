#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"


void *process_transaction (void *arg);
void *update_balance (void *arg);
char *filename;
account **accountArray;
int totalAccounts;
int isBankRunning;
int totalWorkers = 10;
int COUNTER = 0;
pthread_barrier_t workerRendezvous;


threadMediator bankSync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .sig1 = PTHREAD_COND_INITIALIZER,
    .sig2 = PTHREAD_COND_INITIALIZER
};


threadMediator workerSync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .sig1 = PTHREAD_COND_INITIALIZER,
    .sig2 = PTHREAD_COND_INITIALIZER
};


mutexCounter requestCounter = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .count = 0,
};


// MAIN THREAD
int main (int argc, char *argv[]) 
{
    FILE *stream;
    pthread_t workerThreads[10];
    pthread_t bankThread;
    int i;

    /* Validate input. */
    if (argc != 2) {
        fprintf(stderr, USUAGE, ERROR, argv[0]);
        exit(EXIT_FAILURE); 
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {
        printf(STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Extract account data */
    getAccounts(stream, filename, &accountArray, &totalAccounts);
    /* Create the bank thread and Synchronize with it */
    isBankRunning = 1;
    pthread_mutex_lock(&bankSync.lock);
    pthread_create(&bankThread, NULL, update_balance, NULL);
    pthread_cond_wait(&bankSync.sig2, &bankSync.lock);
    pthread_mutex_unlock(&bankSync.lock);
    /* Create worker threads */
    pthread_barrier_init(&workerRendezvous, NULL, 11); 
    pthread_mutex_lock(&workerSync.lock);
    for (i=0; i<totalWorkers; i++) {
        pthread_create(&workerThreads[i], NULL, process_transaction, NULL);
    }
    /* Wait until all workers are created */
    pthread_barrier_wait(&workerRendezvous);
    pthread_cond_wait(&workerSync.sig2, &workerSync.lock);  
    /* Signal all worker threads to start working */
    pthread_cond_broadcast(&workerSync.sig1);
    pthread_mutex_unlock(&workerSync.lock);
    /* Wait until all worker threads exited */
    for (i=0; i<totalWorkers; i++) {
        pthread_join(workerThreads[i], NULL);
    }
    /* Terminate the bank thread */
    isBankRunning = 0;
    pthread_cond_signal(&bankSync.sig1);
    pthread_join(bankThread, NULL);
    /* Output data to standard out */
    print_balances(accountArray, totalAccounts);
    /* Release resources */
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}


// WORKER THREAD
void *process_transaction (void *arg)
{
    FILE *stream;
    int lineOffset;
    char line[BUFSIZ];
    cmd *request;
    int i;
    
    /* Synchronize with Main thread */
    pthread_barrier_wait(&workerRendezvous);
    pthread_mutex_lock(&workerSync.lock);
    lineOffset = COUNTER++;
    if (lineOffset == totalWorkers-1)   
        pthread_cond_signal(&workerSync.sig2);     // Last worker signals main.
    pthread_cond_wait(&workerSync.sig1, &workerSync.lock);    // Wait for main.
    pthread_mutex_unlock(&workerSync.lock);             // unblock next worker.
    /* Move the File pointer into position */
    stream = fopen(filename, "r");
    for (i = 0; i<totalAccounts*5+1; i++)
        fgets(line, BUFSIZ, stream);        // skip the account creation lines.
    for (i = 0; i<lineOffset; i++)
        fgets(line, BUFSIZ, stream);           // skip the next "offset" lines.
    /* Process Requests from the Input File */
    while ((request = readRequest(stream)) != NULL) {
        CommandInterpreter(accountArray, request, totalAccounts);
        freecmd(request);
        for (i = 0; i<totalWorkers-1; i++)
            fgets(line, BUFSIZ, stream);            // Skip the next N-1 lines.
    }
    /* Free resources */
    fclose(stream);
    return NULL;
}


// BANK THREAD
void *update_balance (void *arg)
{
    pthread_mutex_lock(&bankSync.lock);
    pthread_cond_signal(&bankSync.sig2);
    while (isBankRunning) {
        pthread_cond_wait(&bankSync.sig1, &bankSync.lock);
        pthread_cond_signal(&bankSync.sig2);
        ProcessReward(accountArray, totalAccounts);
    }
}
