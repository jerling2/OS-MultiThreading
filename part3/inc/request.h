#ifndef REQUEST_H
#define REQUEST_H


#include <pthread.h>
#include "account.h"
#include "parser.h"


typedef struct mutexCounter {
    int count;
    pthread_mutex_t lock;
} mutexCounter;


typedef struct threadMediator {
    pthread_mutex_t lock;
    pthread_cond_t sig1;
    pthread_cond_t sig2;
} threadMediator;

#if defined(PART3)

    extern threadMediator bankSync;
    extern mutexCounter requestCounter;

    void IncrementCount();

#endif

// void CommandInterpreter(hashmap *hm, cmd *command); 
void CommandInterpreter(account **accountArray, cmd *command,  int totalAccounts);

void ObtainAccountLocks(account *a1, account *a2);

void ReleaseAccountLocks(account *a1, account *a2);

void Transfer(account *acc1, account *acc2, double funds);

void Deposit(account *acc, double funds);

void UpdateTracker(account *acc, double funds);

void Withdraw(account *acc, double funds);

void ProcessReward(account **account_array, int numacs);

void AppendToFile(account *acc);

#endif