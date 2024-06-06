#ifndef BANK_H
#define BANK_H
#include <pthread.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"

#define TOTALWORKERS 10

extern int COUNTER;

extern pthread_barrier_t workerRendezvous;

extern char *filename;

extern account **accountArray;

extern int totalAccounts;

extern int bankIsRunning;

extern int *puddlesIsRunning;

extern threadMediator *memorySync;

void PuddlesDriver();

void InitializeMemorySyncMechanisms();

void *update_balance (void *arg);

void *process_transaction (void *arg);

#endif