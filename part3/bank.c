/*
Joseph Erlinger (jerling2@uoregon.edu)

This is part3 of the project and serves as the main driver for completing the
task outlined by the project description.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"
#define _GNU_SOURCE

// Macro for turning on/off debug messages.
#ifdef DEBUG_ENABLED
#define DEBUG if (1)
#else
#define DEBUG if (0)
#endif

// Macro for turning on/off info messages.
#ifdef INFO_ENABLED
#define INFO if (1)
#else
#define INFO if (0)
#endif

// For notifying invalid usuage.
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"

// For supressing the unused parameter warning.
#define UNUSED(x) (void)(x)

// Configuration for this program.
#define TOTALWORKERS 10


void *process_transaction (void *arg);

void *update_balance (void *arg);


//**************************************************//
/* Accounting Information (private to this process) */

char *filename;

account **accountArray;

int totalAccounts;

mutexCounter requestCounter = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .count = 0,
};

int bankerUpdateCount = 0;

//**************************************************//
/*  Worker Thread Globals (private to this process) */

int COUNTER = 0;

pthread_barrier_t workerRendezvous;  

threadMediator workerSync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .sig1 = PTHREAD_COND_INITIALIZER,
    .sig2 = PTHREAD_COND_INITIALIZER
};

//**************************************************//
/*  Banker Thread Globals (private to this process) */

int bankIsRunning;

threadMediator bankSync = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .sig1 = PTHREAD_COND_INITIALIZER,
    .sig2 = PTHREAD_COND_INITIALIZER
};


/**
 * @brief The main thread.
 * 
 * This function is run by the main thread. The main thread is responsible for
 * creating the banker thread and worker threads. To do this, the main thread
 * uses sychronization mechanisms to ensure that all child threads are
 * initialized and are at the correct position within their thread of execution
 * before the main thread proceeds. The main thread is also responsible for
 * overseeing the termination of child threads. Additionally, the main thread
 * creates global variables to be used by child threads.
 */
int main (int argc, char *argv[]) 
{
    FILE *stream;
    pthread_t workerThreads[TOTALWORKERS];
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
    DEBUG printf("-- Part 3 --\n");
    DEBUG printf("main (pid=%u) is extracting the accounting information.\n", getpid());
    GetAccounts(stream, &accountArray, &totalAccounts);
    /* Create the bank thread and Synchronize with it */
    DEBUG printf("main is creating Duck Bank.\n");
    bankIsRunning = 1;
    pthread_mutex_lock(&bankSync.lock);
    pthread_create(&bankThread, NULL, update_balance, NULL);
    pthread_cond_wait(&bankSync.sig2, &bankSync.lock);
    pthread_mutex_unlock(&bankSync.lock);
    /* Create worker threads and Syncrhonize with them */
    DEBUG printf("main is creating worker threads.\n");
    pthread_barrier_init(&workerRendezvous, NULL, TOTALWORKERS + 1); 
    pthread_mutex_lock(&workerSync.lock);
    for (i=0; i<TOTALWORKERS; i++) {
        pthread_create(&workerThreads[i], NULL, process_transaction, NULL);
    }
    /* Wait until all workers are created */
    pthread_barrier_wait(&workerRendezvous);
    pthread_cond_wait(&workerSync.sig2, &workerSync.lock);  
    /* Signal all worker threads to start working */
    pthread_cond_broadcast(&workerSync.sig1);
    DEBUG printf("main broadcasted a signal to all worker threads.\n");
    pthread_mutex_unlock(&workerSync.lock);
    /* Wait until all worker threads exited */
    for (i=0; i<TOTALWORKERS; i++) {
        pthread_join(workerThreads[i], NULL);
    }
    /* Terminate the bank thread */
    DEBUG printf("main signaled duck bank to exit.\n");
    bankIsRunning = 0;
    pthread_cond_signal(&bankSync.sig1);
    pthread_join(bankThread, NULL);
    /* Release resources */
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}


/**
 * @brief The Worker Threads.
 * 
 * This function is run by multiple worker threads. Each worker thread reads a
 * portion of the input file. After the total number of requests processed
 * reaches a threshold, all but one worker threads will fall asleep. The one
 * worker thread that is awake will wake up the banker thread to do some work.
 * When the banker thread is awake, all worker threads will be asleep. A worker
 * thread that reaches the end of the input file will exit of its own volition.
 */
void *process_transaction (void *arg)
{
    UNUSED(arg);
    FILE *stream;
    int lineOffset;
    char line[BUFSIZ];
    cmd *request;
    int i;
    
    /* Synchronize with Main thread */
    pthread_barrier_wait(&workerRendezvous);
    pthread_mutex_lock(&workerSync.lock);
    lineOffset = COUNTER++;
    DEBUG printf("worker thread #%d (tid=%lu) was created.\n", lineOffset, pthread_self());
    if (lineOffset == TOTALWORKERS-1) {
        pthread_cond_signal(&workerSync.sig2);     // Last worker signals main.
        DEBUG printf("worker thread #%d signaled main.\n", lineOffset);
    }
    DEBUG printf("worker thread #%d is waiting.\n", lineOffset);
    pthread_cond_wait(&workerSync.sig1, &workerSync.lock);    // Wait for main.
    DEBUG printf("worker thread #%d is running!\n", lineOffset);
    pthread_mutex_unlock(&workerSync.lock);             // unblock next worker.
    /* Move the File pointer into position */
    stream = fopen(filename, "r");
    for (i = 0; i<totalAccounts*5+1; i++)
        fgets(line, BUFSIZ, stream);        // skip the account creation lines.
    for (i = 0; i<lineOffset; i++)
        fgets(line, BUFSIZ, stream);           // skip the next "offset" lines.
    /* Process Requests from the Input File */
    while ((request = ReadRequest(stream)) != NULL) {
        CommandInterpreter(accountArray, request, totalAccounts);
        FreeCmd(request);
        for (i = 0; i<TOTALWORKERS-1; i++)
            fgets(line, BUFSIZ, stream);            // Skip the next N-1 lines.
    }
    DEBUG printf("worker thread #%d is done.\n", lineOffset);
    /* Free resources */
    fclose(stream);
    return NULL;
}


/**
 * @brief The Banker Thread.
 * 
 * This function is run by a single banker thread. The banker is woken up by a
 * worker thread when its time to process the rewards for all Duck Bank
 * accounts. The main thread will wake up the banker thread when its time to
 * exit.
 * 
 * The interest is calculated by: 
 *     new_balance += transactionTracker * rewardRate.
 */
void *update_balance (void *arg)
{
    UNUSED(arg);
    pthread_mutex_lock(&bankSync.lock);
    pthread_cond_signal(&bankSync.sig2);
    DEBUG printf("Duck Bank thread (tid=%lu) was created.\n", pthread_self());
    while (bankIsRunning) {
        pthread_cond_wait(&bankSync.sig1, &bankSync.lock);
        pthread_cond_signal(&bankSync.sig2);
        if (!bankIsRunning) break;       // Note: Bank will exit with the lock.
        ProcessReward(accountArray, totalAccounts);
        DEBUG printf("Duck    Bank received signal, update count = %d\n", bankerUpdateCount+++1);
    }
    /* Output data to standard out */
    DEBUG printf("\n");
    INFO printf("Duck Bank Balances:\n");
    INFO PrintBalances(accountArray, totalAccounts);
    DEBUG printf("\n");
    DEBUG printf("Duck Bank is done. Total updates = %d\n", bankerUpdateCount);
    return NULL;
}
