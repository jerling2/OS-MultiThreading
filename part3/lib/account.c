/*
Joseph Erlinger (jerling2@uoregon.edu)

This file "account.c" is responsible for initializing accounts and managing an
array of accounts.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"


account *InitAccount(char *id, char *pass, char *outfile, double balance, 
    double rewardRate, int order)
{
    FILE *logfile;
    account *newAccount;

    newAccount = (account *)malloc(sizeof(account));
    newAccount->accountNumber = strdup(id);
    newAccount->password = strdup(pass);
    newAccount->outFile = strdup(outfile);
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


void FreeAccount(account *account)
{
    free(account->accountNumber);
    free(account->password);
    free(account->outFile);
    free(account);
}


void FreeAccountArray(account **accountArray, int arraySize)
{
    int i;

    for(i=0; i<arraySize; i++){
        FreeAccount(accountArray[i]);
    }
    free(accountArray);
}


account *Find(account **accountArray, char *accountID, int arraySize)
{
    int i;

    for(i=0; i<arraySize; i++) {
        if(strcmp(accountArray[i]->accountNumber, accountID) == 0)
            return accountArray[i];
    } 
    return NULL;
}


void PrintBalances(account **accountArray, int totalAccounts)
{
    int i;

    for (i = 0; i<totalAccounts; i++)
        printf("%d balance:\t%.2f\n", i, accountArray[i]->balance);
    return;
}

