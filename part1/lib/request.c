/*
Joseph Erlinger (jerling2@uoregon.edu)

This file "request.c" is for processing request 'cmd' structs. Some functions
change in functionality depending on the part#.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "account.h"
#include "parser.h"
#include "request.h"

// Macro for turning on/off debug messages.
#ifdef DEBUG_ENABLED
#define DEBUG if (1)
#else
#define DEBUG if (0)
#endif

/**
 * @brief Process a request represented by a cmd structure.
 * 
 * This function slightly changes depending on the part#.
 *   For part1: No multi-threading and no synchronization with the bank thread.
 *   For part2: Multi-threading but no synchronization with the bank thread.
 *   For part3: Multi-threading and synchronization with the bank thread.
 * 
 * @param[in] accountArray (account **) accountArray to search for accounts.
 * @param[in] request (cmd *) A request to be processed.
 * @param[in] totalAccounts (int) the size of the accountArray.
 */
void CommandInterpreter(account **accountArray, cmd *request,  int totalAccounts)
{
    char *operator;     // The type of request indicated by the first argument.
    char **argv;        // Alias for the request's argv.
    account *a1, *a2;   // Account pointers (a2 can be NULL).
    int totalOperands;  // The number of arguments in argv - 2.
    double funds;       // The funds specified by the request.  

    operator = request->argv[0];
    argv = request->argv;
    totalOperands = request->size - 2;   // -2 for the operator and NULL token.
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
    #if defined(PART2) || defined(PART3) || defined(PART4)
    ObtainAccountLocks(a1, a2);     // Obtain locks based on a priority system.
    #endif
    #if defined(PART3) || defined(PART4)
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
    #if defined(PART3) || defined(PART4)
    IncrementCount();
    pthread_mutex_unlock(&requestCounter.lock);
    #endif
    #if defined(PART2) || defined(PART3) || defined(PART4)
    ReleaseAccountLocks(a1, a2);          // Release locks in reverse priority.
    #endif 
}


/**
 * @brief Increment the request counter and wakeup the bank if necessary.
 * 
 * This function is used to wakeup the bank after a total of 5000 requests have
 * been processed across all worker threads. 
 * 
 * Dependency: This function is dependent on the global threadMediator bankSync
 * variable. The bankSync variable should be defined in the main driver. This is
 * why this function is only defined for Part3 and Part4.
 */
#if defined(PART3) || defined(PART4)
void IncrementCount()
{
    pthread_mutex_lock(&bankSync.lock);
    requestCounter.count ++;
    if (requestCounter.count == 5000) {
        DEBUG printf("Reached 5000 request!\n");
        DEBUG printf("Worker (tid=%lu) signaled bank\n", pthread_self());
        pthread_cond_signal(&bankSync.sig1);
        pthread_cond_wait(&bankSync.sig2, &bankSync.lock);
        requestCounter.count = 0;
    }
    pthread_mutex_unlock(&bankSync.lock);
}
#endif


/**
 * @brief Obtain account locks based on an priority system.
 * 
 * Account locks are retrieved in increasing order to prevent circular wait.
 * Without this mechanism, deadlocks can occur.
 * 
 * Note: a2 can be NULL, but not a1. 
 * 
 * @param[in] a1 (account *) Pointer to an account.
 * @param[in] command (account *) Either a pointer to an account or NULL.
 */
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


/**
 * @brief Release account locks based on an priority system.
 * 
 * Account locks are released in decreasing order to prevent circular wait.
 * Without this mechanism, deadlocks can occur.
 * 
 * Note: a2 can be NULL, but not a1. 
 * 
 * @param[in] a1 (account *) Pointer to an account.
 * @param[in] command (account *) Either a pointer to an account or NULL.
 */
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


/**
 * @brief Transfer funds from account 1 to account 2.
 * 
 * @param[in] a1 (account *) Account 1.
 * @param[in] a2 (account *) Account 2.
 * @param[in] funds (double) Funds to be transfered.
 */
void Transfer(account *a1, account *a2, double funds)
{
    Withdraw(a1, funds);
    Deposit(a2, funds);
}


/**
 * @brief Withdraw funds from an account.
 * 
 * @param[in] account (account *) Account.
 * @param[in] funds (double) Funds to be withdrawed.
 */
void Withdraw(account *account, double funds)
{
    account->balance -= funds;
}


/**
 * @brief Deposit funds to an account.
 * 
 * @param[in] account (account *) Account.
 * @param[in] funds (double) Funds to be deposited.
 */
void Deposit(account *account, double funds)
{
    account->balance += funds;
}


/**
 * @brief Update the transactionTracker of an account.
 * 
 * This function is called whenever an account initiates a transfer, withdraw
 * or deposit. The transactionTracker is used by ProcessReward to apply give
 * each account a reward based on their rewardRate. 
 * 
 * @param[in] account (account *) Account.
 * @param[in] funds (double) Funds to be added to the transactionTracker.
 */
void UpdateTracker(account *account, double funds)
{
    account->transactionTracker += funds;
}


/**
 * @brief Give each account an reward to their balance based on their 
 * transcationTracker and rewardRate.
 * 
 * This function is called by the bank thread (in parts 2, 3, and 4) or by the
 * main thread in part 1. Even though there is no locks surrounding the write
 * operation, no race condition can occur. Since, in parts 2, the bank thread 
 * is run only after all worker threads have exited. Then, in parts 3 and 4, 
 * all worker threads are blocked by other mechanisms (for example, in 
 * UpdateTracker). Therefore, this function safe in context with how its used
 * in the system.
 * 
 * Note: Valgrind tool, helgrind, will report an error in this function.
 * However, this function is safe in context with how its used in the system.
 * 
 * @param[in] accountArray (account **).
 * @param[in] totalAccounts (int).
 */
void ProcessReward(account **accountArray, int totalAccounts)
{
    double reward;   // The reward that will be added to the account's balance.
    int i;           // The current account.

    for (i = 0; i<totalAccounts; i++) {
        reward  = accountArray[i]->transactionTracker;
        reward *= accountArray[i]->rewardRate;
        accountArray[i]->balance += reward;
        accountArray[i]->transactionTracker = 0; //< Helgrind error.
        AppendToFile(accountArray[i], 1);
    }
}


/**
 * @brief Update each account's saving by applying a flat rewardRate on their balance.
 * 
 * This function is very similar to ProcessReward with the difference being that here
 * we do not care about the transactionTracker.
 * 
 * @param[in] accountArray (account **).
 * @param[in] totalAccounts (int).
*/
void UpdateSavings(account **accountArray, int totalAccounts)
{
    double reward;   // The reward that will be added to the account's balance.
    int i;           // The current account.

    for (i = 0; i<totalAccounts; i++) {
        reward = accountArray[i]->balance;
        reward *= accountArray[i]->rewardRate;
        accountArray[i]->balance += reward;
        AppendToFile(accountArray[i], 0);
    }
}


/**
 * @brief Append an account's balance to the accounts outFile.
 * 
 * ProcessReward and UpdateSavings calls this function after an account
 * receives its reward. That is why I chose to keep this function in this file
 * instead of being in fileio.c
 * 
 * @param[in] account (account *) Account.
 * @return balance data appended to the account's outFile.
 */
void AppendToFile(account *account, int flag)
{
    FILE *stream;    // The stream of the account's outFile.

    stream = fopen(account->outFile, "a");
    if (flag) fprintf(stream, "balance: %.2f\n", account->balance);
    else      fprintf(stream, "savings: %.2f\n", account->balance);
    fflush(stream);
    fclose(stream);
}