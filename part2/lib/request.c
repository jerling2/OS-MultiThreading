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
    char *operator;
    char **argv;
    account *a1, *a2;
    int totalOperands;
    double funds;

    operator = command->argv[0];
    argv = command->argv;
    totalOperands = command->size - 2;
    a1 = Find(accountArray, argv[1], totalAccounts);
    if (strcmp(a1->password, argv[2]) != 0) { // Check password
        return;
    }
    if (totalOperands==4) {
        a2 = Find(accountArray, argv[3], totalAccounts);
        funds = strtod(argv[4], NULL);
    } else
    if (totalOperands==3) {
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
    if (strcmp(operator, "T") == 0) {
        Transfer(a1, a2, funds);
        UpdateTracker(a1, funds);
    } else 
    if (strcmp(operator, "W") == 0) {
        Withdraw(a1, funds);
        UpdateTracker(a1, funds);
    } else
    if (strcmp(operator, "D") == 0) {
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
        pthread_mutex_lock(&a1->lock);
        return;
    }
    if (a1->priority < a2->priority) {
        pthread_mutex_lock(&a1->lock);
        pthread_mutex_lock(&a2->lock);
    } else {
        pthread_mutex_lock(&a2->lock);
        pthread_mutex_lock(&a1->lock);
    }
    return;      
}


void ReleaseAccountLocks(account *a1, account *a2)
{
    if (a2 == NULL) {
        pthread_mutex_unlock(&a1->lock);
        return;
    }
    if (a1->priority > a2->priority) {
        pthread_mutex_unlock(&a1->lock);
        pthread_mutex_unlock(&a2->lock);
    } else {
        pthread_mutex_unlock(&a2->lock);
        pthread_mutex_unlock(&a1->lock);
    }
    return;      
}


void Transfer(account *a1, account *a2, double funds)
{
    Withdraw(a1, funds);
    Deposit(a2, funds);
}


void Withdraw(account *account, double funds)
{
    account->balance -= funds;
}


void Deposit(account *account, double funds)
{
    account->balance += funds;
}


void UpdateTracker(account *account, double funds)
{
    account->transactionTracker += funds;
}


void ProcessReward(account **accountArray, int totalAccounts)
{
    double reward;
    int i;

    for (i = 0; i<totalAccounts; i++) {
        reward = accountArray[i]->transactionTracker;
        reward *= accountArray[i]->rewardRate;
        accountArray[i]->balance += reward;
        accountArray[i]->transactionTracker = 0;            //< Helgrind error.
        AppendToFile(accountArray[i]);
    }
}


void AppendToFile(account *account)
{
    FILE *stream;

    stream = fopen(account->outFile, "a");
    fprintf(stream, "balance: %.2f\n", account->balance);
    fflush(stream);
    fclose(stream);
}