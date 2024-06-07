/*
Joseph Erlinger (jerling2@uoregon.edu)

This file "account.c" is responsible for initializing accounts and managing an
array of accounts.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include "account.h"


/**
 * @brief Creates a new account structure.
 * 
 * This function initializes and returns a new account structure by taking 
 * several input parameters and packaging them together.
 * 
 * @param[in] id (char *) A unique 17-byte long numerical ID for the account.
 * @param[in] pass (char *) A 9-byte long string.
 * @param[in] outfile (char *) A 64-byte file path.
 * @param[in] balance (double) The initial balance of the account.
 * @param[in] rewardRate (double) The reward rate for the account.
 * @param[in] order (int) Determines when the account lock can be aquired.
 * @return A new account structure and a file specified by the "outfile" param.
 */
account *InitAccount(char *id, char *pass, char *outfile, double balance, 
    double rewardRate, int order)
{
    FILE *logfile;          // The path of the account's associated log file.
    account *newAccount;    // A pointer to the newly created account.

    newAccount = (account *)malloc(sizeof(account));
    newAccount->accountNumber = strdup(id); // malloc-d
    newAccount->password = strdup(pass);  // malloc-d
    newAccount->outFile = strdup(outfile);  // malloc-d
    newAccount->balance = balance;
    newAccount->rewardRate = rewardRate;
    newAccount->transactionTracker = 0;
    newAccount->priority = order;
    if (pthread_mutex_init(&(newAccount->lock), NULL) != 0) {
        free(newAccount); // Free memory if mutex initialization fails
        return NULL;
    }
    logfile = fopen(outfile, "w");
    fclose(logfile);
    return newAccount;
}


/**
 * @brief Free an account structure.
 * 
 * @param[in] account (account *). The account to be freed.
 */
void FreeAccount(account *account)
{
    free(account->accountNumber);
    free(account->password);
    free(account->outFile);
    free(account);
}


/**
 * @brief Free an array of accounts.
 * 
 * @param[in] accountArray (account **) The array of accounts to be freed.
 * @param[in] arraySize (int) The size of the account array.
 */
void FreeAccountArray(account **accountArray, int arraySize)
{
    int i;

    for(i=0; i<arraySize; i++){
        FreeAccount(accountArray[i]);
    }
    free(accountArray);
}


/**
 * @brief Lookup an account by its ID/Account Number.
 * 
 * This function traverses an account array and returns the account associated
 * with the 'accountID'.
 * 
 * @param[in] accountArray (account **) The array of accounts to search in.
 * @param[in] accountID (char *) The unique ID/Number of an account.
 * @param[in] arraySize (int) The size of the accountArray.
 * @return A pointer to the account associated with the inputted accountID.
 */
account *Find(account **accountArray, char *accountID, int arraySize)
{
    int i;

    for(i=0; i<arraySize; i++) {
        if(strcmp(accountArray[i]->accountNumber, accountID) == 0)
            return accountArray[i];
    } 
    return NULL;
}


/**
 * @brief Print the balance of each account to standard output.
 * 
 * @param[in] stream (FILE *) The stream to output the print data.
 * @param[in] accountArray (account **) An array of account structs.
 * @param[in] arraySize (int) The size of the accountArray.
 */
void PrintBalances(FILE *stream, account **accountArray, int totalAccounts)
{
    int i;

    for (i = 0; i<totalAccounts; i++) {
        fprintf(stream, "%d balance:\t%.2f\n", i, accountArray[i]->balance);
        fflush(stream);
    }
    return;
}

