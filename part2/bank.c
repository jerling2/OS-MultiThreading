#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "fileio.h"
// #include "list.h"
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
pthread_mutex_t workerLock = PTHREAD_MUTEX_INITIALIZER;


int main (int argc, char *argv[]) 
{
    FILE *stream;
    pthread_t worker_threads[10];
    pthread_t banker_thread;
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
    /* Extract account data and Create datastructures */
    getAccounts(stream, filename, &accountArray, &totalAccounts);
    /* Create worker threads */
    for (i=0; i<totalWorkers; i++) {
        pthread_create(&worker_threads[i], NULL, process_transaction, NULL);
    }
    /* Wait until all worker threads exited */
    for (i=0; i<totalWorkers; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    /* Create the bank thread just for it to call ProcessReward */
    pthread_create(&banker_thread, NULL, update_balance, NULL);
    pthread_join(banker_thread, NULL);
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
    
    pthread_mutex_lock(&workerLock);
    lineOffset = COUNTER++;
    pthread_mutex_unlock(&workerLock);
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
    ProcessReward(accountArray, totalAccounts);
}