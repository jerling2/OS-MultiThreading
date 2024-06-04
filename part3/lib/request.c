#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "account.h"
#include "parser.h"
#include "request.h"


void CommandInterpreter(account **accountArray, cmd *command,  int totalAccounts)
{
    char *op;
    char **argv;
    account *a1, *a2;
    int numarg;
    double funds;

    op = command->argv[0];
    argv = command->argv;
    numarg = command->size - 2;
    a1 = Find(accountArray, argv[1], totalAccounts);
    if (strcmp(a1->password, argv[2]) != 0) { // Check password
        return;
    }
    if (numarg==4) {
        a2 = Find(accountArray, argv[3], totalAccounts);
        funds = strtod(argv[4], NULL);
    } else
    if (numarg==3) {
        a2 = NULL;
        funds = strtod(argv[3], NULL);
    } else {
        return; // Check request does nothing.
    }
    #if defined(PART2) || defined(PART3)
    ObtainAccountLocks(a1, a2);
    #endif
    #ifdef PART3
    pthread_mutex_lock(&requestCounter.lock);
    #endif
    if (strcmp(op, "T") == 0) {
        Transfer(a1, a2, funds);
        UpdateTracker(a1, funds);
    } else 
    if (strcmp(op, "W") == 0) {
        Withdraw(a1, funds);
        UpdateTracker(a1, funds);
    } else
    if (strcmp(op, "D") == 0) {
        Deposit(a1, funds);
        UpdateTracker(a1, funds);
    }
    #ifdef PART3
    IncrementCount();
    pthread_mutex_unlock(&requestCounter.lock);
    #endif
    #if defined(PART2) || defined(PART3)
    ReleaseAccountLocks(a1, a2);
    #endif 
}


#ifdef PART3
void IncrementCount()
{
    pthread_mutex_lock(&bankSync.lock);
    requestCounter.count ++;
    if (requestCounter.count == 5000) {
        pthread_cond_signal(&bankSync.sig1);
        pthread_cond_wait(&bankSync.sig2, &bankSync.lock);
        requestCounter.count = 0;
    }
    pthread_mutex_unlock(&bankSync.lock);
}
#endif


void ObtainAccountLocks(account *a1, account *a2)
{
    if (a2 == NULL) {
        pthread_mutex_lock(&a1->ac_lock);
        return;
    }
    if (a1->order < a2->order) {
        pthread_mutex_lock(&a1->ac_lock);
        pthread_mutex_lock(&a2->ac_lock);
    } else {
        pthread_mutex_lock(&a2->ac_lock);
        pthread_mutex_lock(&a1->ac_lock);
    }
    return;      
}


void ReleaseAccountLocks(account *a1, account *a2)
{
    if (a2 == NULL) {
        pthread_mutex_unlock(&a1->ac_lock);
        return;
    }
    if (a1->order > a2->order) {
        pthread_mutex_unlock(&a1->ac_lock);
        pthread_mutex_unlock(&a2->ac_lock);
    } else {
        pthread_mutex_unlock(&a2->ac_lock);
        pthread_mutex_unlock(&a1->ac_lock);
    }
    return;      
}


void Transfer(account *acc1, account *acc2, double funds)
{
    Withdraw(acc1, funds);
    Deposit(acc2, funds);
}


void Withdraw(account *acc, double funds)
{
    acc->balance -= funds;
}


void Deposit(account *acc, double funds)
{
    acc->balance += funds;
}


void UpdateTracker(account *acc, double funds)
{
    acc->transaction_tracker += funds;
}


void ProcessReward(account **accountArray, int totalAccounts)
{
    double reward;
    int i;

    for (i = 0; i<totalAccounts; i++) {
        reward = accountArray[i]->transaction_tracker;
        reward *= accountArray[i]->reward_rate;
        accountArray[i]->balance += reward;
        accountArray[i]->transaction_tracker = 0;           //< Helgrind error.
        AppendToFile(accountArray[i]);
    }
}


void AppendToFile(account *acc)
{
    FILE *stream;

    stream = fopen(acc->out_file, "a");
    fprintf(stream, "balance: %.2f\n", acc->balance);
    fflush(stream);
    fclose(stream);
}