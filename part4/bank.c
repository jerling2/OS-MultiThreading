/*
Joseph Erlinger (jerling2@uoregon.edu)

This is part4 of the project and serves as the main driver(s) for completing
the task outlined by the project description.
*/
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
#define SAVINGSFILE "savings/account_%d.log" // Will segfault if directory dne!


void PuddlesDriver();

void InitializeMemorySyncMechanisms();

void *update_balance (void *arg);

void *process_transaction (void *arg);


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

int puddlesUpdateCount = 0;

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

//**************************************************//
/*   PuddlesBank Global (shared to all processes)   */

int *puddlesIsRunning;

threadMediator *memorySync;


/**
 * @brief The main thread.
 * 
 * This function is run by the main thread. The main thread is responsible for
 * creating the banker thread, worker threads, and the Puddles Bank process. To
 * do this, the main thread uses sychronization mechanisms to ensure that all
 * child threads/processes are initialized and are at the correct position 
 * within their thread of execution before the main thread proceeds. The main
 * thread is also responsible for overseeing the termination of child threads
 * and child processes. Additionally, the main thread creates global variables
 * to be used by child threads or shared across processes.
 */
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
        fprintf(stderr, STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Extract accounting data */
    DEBUG printf("-- Part 4 --\n");
    DEBUG printf("main (pid=%u) is extracting the accounting information.\n", getpid());
    GetAccounts(stream, &accountArray, &totalAccounts);
    /* Create Puddles Savings Bank and Synchronize with it */
    DEBUG printf("main is creating Puddles Bank.\n");
    InitializeMemorySyncMechanisms();
    *puddlesIsRunning = 1;
    pthread_mutex_lock(&memorySync->lock);
    if ((pid = fork()) == 0) {
        /* This code is executed by Puddles Bank */
        fclose(stream);
        PuddlesDriver();
    }
    pthread_cond_wait(&memorySync->sig2, &memorySync->lock);
    pthread_mutex_unlock(&memorySync->lock);
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
    /* Terminate Puddles Bank */
    DEBUG printf("main signaled puddles bank to exit.\n");
    *puddlesIsRunning = 0;
    pthread_cond_signal(&memorySync->sig1);
    waitpid(pid, NULL, 0);
    /* Release resources */
    munmap(puddlesIsRunning, sizeof(int));
    munmap(memorySync, sizeof(threadMediator));
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}


/**
 * @brief The Puddles Bank Process.
 * 
 * This function is run by the Puddles Bank process. The Puddles Bank process
 * synchronizes with the main process by using synchronization mechanisms that
 * are in shared memory. The Puddles Bank process starts with copy of the
 * intial accounting information. It then reduces each account's inital balance
 * to 20% of its original value, sets all account's reward rate to 2%, and sets
 * the outfile directory to "savings". Everytime the the banker thread in the
 * main process applies interest to the Duck Bank accounts, it will tell the 
 * Puddles Bank process to apply the saving interest to the Puddles Bank
 * accounts. The main process will wake up the Puddles Bank process when its
 * time to exit.
 * 
 * The saving interest is calculated by: new_balance += old_balance * 0.02
 * 
 * Note: the transactionTracker is not used to calculate the savings interest.
 */
void PuddlesDriver()
{
    FILE *savingsFile;    // The path of the account's associated savings file.
    char outfile[64];     // Temp buffer to hold the new outfile path.
    int i;                // The current account (between 0 and totalAccounts).

    pthread_mutex_lock(&memorySync->lock);
    pthread_cond_signal(&memorySync->sig2);
    DEBUG printf("Puddles Bank (pid=%u) was created.\n", getpid());
    /* Modify Account Information to match Savings Account Specification */
    for (i=0;i<totalAccounts;i++) {
        accountArray[i]->balance *= 0.20;   // Inital balance = 20% of orignal.
        accountArray[i]->rewardRate = 0.02;   // Flat reward rate for all accs.
        /* Change outfile directory to the savings direction */
        snprintf(outfile, 64, SAVINGSFILE, i);                // format string.
        free(accountArray[i]->outFile);                   // Free old filepath.
        accountArray[i]->outFile = strdup(outfile);    // Assign new file path.
        savingsFile = fopen(outfile, "w");             // Create/truncate file.
        fclose(savingsFile);
    }
    /* Initialization is complete at this point. */
    /* Now we enter the main loop */
    while (*puddlesIsRunning) {
        pthread_cond_wait(&memorySync->sig1, &memorySync->lock);
        pthread_cond_signal(&memorySync->sig2);
        if (!*puddlesIsRunning) break;// Note: Puddles will exit with the lock.
        UpdateSavings(accountArray, totalAccounts);
        DEBUG printf("Puddles Bank received signal, update count = %d\n", puddlesUpdateCount+++1);
    }
    /* Print savings to standard output. */
    DEBUG printf("\n");
    INFO printf("Puddles Bank Balances:\n");
    INFO PrintBalances(accountArray, totalAccounts);
    DEBUG printf("\n");
    DEBUG printf("Puddles Bank is done. Total updates = %d\n", puddlesUpdateCount);
    /* Free resources and exit. */
    FreeAccountArray(accountArray, totalAccounts);
    exit(EXIT_SUCCESS);
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
    FILE *stream;       // The file pointer of the input file.
    int lineOffset;     // The portion of the input file this thread will read.
    char line[BUFSIZ];  // Temp buffer to hold the line read from the file.
    cmd *request;    // The tokenized request to pass to the CommandInterpreter.
    int i;
    
    /* Synchronize with Main thread */
    pthread_barrier_wait(&workerRendezvous);
    pthread_mutex_lock(&workerSync.lock);
    lineOffset = COUNTER++;// Assign this thread's portion of the file to read.
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
 * accounts (i.e. not the Puddles Bank accounts). The main thread will wake up
 * the banker thread when its time to exit.
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
        pthread_mutex_lock(&memorySync->lock);
        pthread_cond_signal(&memorySync->sig1);
        pthread_cond_wait(&memorySync->sig2, &memorySync->lock);
        pthread_mutex_unlock(&memorySync->lock);
        ProcessReward(accountArray, totalAccounts);
        DEBUG printf("Duck    Bank received signal, update count = %d\n", bankerUpdateCount+++1);
    }
    /* Output data to standard out */
    DEBUG printf("\n");
    INFO printf("Duck Bank Balances:\n");
    INFO PrintBalances(accountArray, totalAccounts);
    INFO printf("\n");
    DEBUG printf("Duck Bank is done. Total updates = %d\n", bankerUpdateCount);
    return NULL;
}


/**
 * @brief Initialize synchronization mechanisms within shared memory.
 * 
 * This function creates two synchronization mechanisms to help synchronize the
 * main process and the puddles bank process. The first synchronization tool is
 * the shared threadMediator. The locks and conditions of the threadMediator
 * needs to be dynamically initialized with the PTHREAD_PROCESS_SHARED
 * attribute. The second synchronization tool is the shared boolean flag. This
 * flag is used by the main process to tell the puddles bank process to exit
 * out of its main loop.
 * 
 * Note: The synchronization tools exist as global variables to both processes.
 */
void InitializeMemorySyncMechanisms()
{
    pthread_mutexattr_t mutex_attr; // To hold the PTHREAD_PROCESS_SHARED attr.
    pthread_condattr_t cond_attr;   // To hold the PTHREAD_PROCESS_SHARED attr.

    /* Set up the shared memory segment */
    memorySync = (threadMediator *) mmap (
        NULL, 
        sizeof(threadMediator),
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_SHARED, 
        -1, 
        0
    );
    puddlesIsRunning = (int *) mmap (
        NULL, 
        sizeof(int), 
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_SHARED,
        -1,
        0
    );
    /* Verify that shared memory segments were created */
    if (memorySync == MAP_FAILED ||
        puddlesIsRunning == MAP_FAILED) {
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
