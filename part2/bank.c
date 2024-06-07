/*
Joseph Erlinger (jerling2@uoregon.edu)

This is part2 of the project and serves as the main driver for completing the
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

//**************************************************//
/*  Worker Thread Globals (private to this process) */

int COUNTER = 0;

pthread_mutex_t workerLock = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief The main thread.
 * 
 * This function is run by the main thread. The main thread is responsible for
 * creating the banker thread and worker threads. The main thread is also
 * responsible for overseeing the termination of child threads. Additionally,
 * the main thread creates global variables to be used by child threads.
 */
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
    /* Extract account data */
    DEBUG printf("-- Part 2 --\n");
    DEBUG printf("main (pid=%u) is extracting the accounting information.\n", getpid());
    GetAccounts(stream, &accountArray, &totalAccounts);
    /* Create worker threads */
    DEBUG printf("main is creating worker threads.\n");
    for (i=0; i<TOTALWORKERS; i++) {
        pthread_create(&worker_threads[i], NULL, process_transaction, NULL);
    }
    /* Wait until all worker threads exited */
    for (i=0; i<TOTALWORKERS; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    /* Create the bank thread just for it to call ProcessReward */
    DEBUG printf("main is creating Duck Bank.\n");
    pthread_create(&banker_thread, NULL, update_balance, NULL);
    pthread_join(banker_thread, NULL);
    /* Output data to standard out */
    DEBUG printf("\n");
    INFO printf("Duck Bank Balances:\n");
    INFO PrintBalances(stdout, accountArray, totalAccounts);
    /* Create "output/output.txt" */
    WriteFinalBalances(accountArray, totalAccounts);
    /* Release resources */ 
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}


/**
 * @brief The Worker Threads.
 * 
 * This function is run by multiple worker threads. Each worker thread reads a
 * portion of the input file. All worker threads exit before the banker thread
 * is created. A worker thread that reaches the end of the input file will exit
 * of its own volition.
 */
void *process_transaction (void *arg)
{
    UNUSED(arg);
    FILE *stream;
    int lineOffset;
    char line[BUFSIZ];
    cmd *request;
    int i;
    
    pthread_mutex_lock(&workerLock);
    lineOffset = COUNTER++;
    DEBUG printf("worker thread #%d (tid=%lu) was created.\n", lineOffset, pthread_self());
    pthread_mutex_unlock(&workerLock);
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
 * This function is run by a single banker thread. The banker thread is created
 * after all worker threads have exited. The banker thread processes the
 * rewards for all Duck Bank accounts, then exits of its own volition.
 * 
 * The interest is calculated by: 
 *     new_balance += transactionTracker * rewardRate.
 */
void *update_balance (void *arg)
{
    UNUSED(arg);
    ProcessReward(accountArray, totalAccounts);
    DEBUG printf("Duck Bank is done.\n");
    return NULL;
}