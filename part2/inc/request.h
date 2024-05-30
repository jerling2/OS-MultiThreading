#ifndef REQUEST_H
#define REQUEST_H
#include <pthread.h>
#include "list.h"
#include "account.h"
#include "parser.h"

typedef struct requestCounter {
    int count;
    pthread_mutex_t rc_lock;
} requestCounter;

typedef struct threadMediator {
    pthread_mutex_t sync_lock;
    pthread_cond_t cond1;
    pthread_cond_t cond2;
} threadMediator;

extern threadMediator worker_bank_sync;

#ifdef SINGLE_THREAD
int commandInterpreter(hashmap *hm, cmd *command);
#elif defined(MULTI_THREAD)
int commandInterpreter(hashmap *hm, cmd *command, requestCounter *rc);
#endif

int verify_password(account *acc, char *pass);

void transfer(account *acc1, account *acc2, double funds);

void deposit(account *acc, double funds);

void update_tracker(account *acc, double funds);

void withdraw(account *acc, double funds);

void check(account *acc);

void process_reward(account **account_array, int numacs);

void appendToFile(account *acc);

void incrementCount(requestCounter *rc);

#endif