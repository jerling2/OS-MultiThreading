#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"
#define _GNU_SOURCE
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"
#define TOTALWORKERS 10

void PuddlesDriver();

void InitializeMemorySyncMechanisms();

void *update_balance (void *arg);

void *process_transaction (void *arg);

int COUNTER = 0;

pthread_barrier_t workerRendezvous;

char *filename;

account **accountArray;

int totalAccounts;

int bankIsRunning;

int *puddlesIsRunning;

threadMediator *memorySync;

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
    pthread_t workerThreads[TOTALWORKERS];
    pthread_t bankThread;
    pid_t pid;
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

    ////////////////////////////////////////////////////////////////////////
    GetAccounts(stream, &accountArray, &totalAccounts);
    InitializeMemorySyncMechanisms();
    *puddlesIsRunning = 1;
    pthread_mutex_lock(&memorySync->lock);
    if ((pid = fork()) == 0) {
        fclose(stream);
        PuddlesDriver();
    }
    pthread_cond_wait(&memorySync->sig2, &memorySync->lock);
    pthread_mutex_unlock(&memorySync->lock);
    ////////////////////////////////////////////////////////////////////////
    /* Create the bank thread and Synchronize with it */
    bankIsRunning = 1;
    pthread_mutex_lock(&bankSync.lock);
    pthread_create(&bankThread, NULL, update_balance, NULL);
    pthread_cond_wait(&bankSync.sig2, &bankSync.lock);
    pthread_mutex_unlock(&bankSync.lock);
    /* Create worker threads */
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
    pthread_mutex_unlock(&workerSync.lock);
    /* Wait until all worker threads exited */
    for (i=0; i<TOTALWORKERS; i++) {
        pthread_join(workerThreads[i], NULL);
    }
    /* Terminate the bank thread */
    bankIsRunning = 0;
    pthread_cond_signal(&bankSync.sig1);
    pthread_join(bankThread, NULL);
     /* Terminate Puddles Bank */
    *puddlesIsRunning = 0;
    pthread_cond_signal(&memorySync->sig1);
    fprintf(stderr, "Test\n");
    waitpid(pid, NULL, 0);
    fprintf(stderr, "Puddles terminated\n");
    /* Output data to standard out */
    PrintBalances(accountArray, totalAccounts);
    /* Release resources */
    munmap(puddlesIsRunning, sizeof(int));
    munmap(memorySync, sizeof(threadMediator));
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}


void PuddlesDriver()
{
    int i;

    pthread_mutex_lock(&memorySync->lock);
    pthread_cond_signal(&memorySync->sig2);
    for (i=0;i<totalAccounts;i++) {
        accountArray[i]->balance *= 0.20;
        accountArray[i]->rewardRate = 0.02;
    }
    while (*puddlesIsRunning) {
        pthread_cond_wait(&memorySync->sig1, &memorySync->lock);
        pthread_cond_signal(&memorySync->sig2);
        // ProcessReward(accountArray, totalAccounts);
    }
    PrintBalances(accountArray, totalAccounts);
    FreeAccountArray(accountArray, totalAccounts);
    exit(EXIT_SUCCESS);
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
    if (lineOffset == TOTALWORKERS-1)   
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
    while ((request = ReadRequest(stream)) != NULL) {
        CommandInterpreter(accountArray, request, totalAccounts);
        FreeCmd(request);
        for (i = 0; i<TOTALWORKERS-1; i++)
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
    while (bankIsRunning) {
        pthread_cond_wait(&bankSync.sig1, &bankSync.lock);
        pthread_cond_signal(&bankSync.sig2);
        ProcessReward(accountArray, totalAccounts);
    }
}


void InitializeMemorySyncMechanisms()
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    /* Set up the shared memory segment */
    memorySync = (threadMediator *) mmap (NULL, sizeof(threadMediator), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (memorySync == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    puddlesIsRunning = (int *) mmap (NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (puddlesIsRunning == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    /* Initialize the thread locks and conditions */
    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&memorySync->lock, &mutex_attr); 
    pthread_cond_init(&memorySync->sig1, &cond_attr);
    pthread_cond_init(&memorySync->sig2, &cond_attr);
}
