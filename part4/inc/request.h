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

#if defined(PART3) || defined(PART4)

    extern threadMediator bankSync;
    extern mutexCounter requestCounter;

    void IncrementCount();

#endif

void CommandInterpreter(account **accountArray, cmd *request,  int totalAccounts);

void ObtainAccountLocks(account *a1, account *a2);

void ReleaseAccountLocks(account *a1, account *a2);

void Transfer(account *a1, account *a2, double funds);

void Deposit(account *account, double funds);

void UpdateTracker(account *account, double funds);

void Withdraw(account *account, double funds);

void ProcessReward(account **account_array, int numacs);

void UpdateSavings(account **accountArray, int totalAccounts);

#endif