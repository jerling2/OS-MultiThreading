#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"


account *InitAccount(char *id, char *pass, char *outfile, double balance, 
    double reward_rate, int order)
{
    FILE *logfile;
    account *newacc;

    newacc = (account *)malloc(sizeof(account));
    newacc->account_number = strdup(id);
    newacc->password = strdup(pass);
    newacc->out_file = strdup(outfile);
    newacc->balance = balance;
    newacc->reward_rate = reward_rate;
    newacc->transaction_tracker = 0;
    newacc->order = order;
    if (pthread_mutex_init(&(newacc->ac_lock), NULL) != 0) {
        free(newacc); // Free memory if mutex initialization fails
        return NULL;
    }
    logfile = fopen(outfile, "w");
    fclose(logfile);
    return newacc;
}


void FreeAccount(account *acc)
{
    free(acc->account_number);
    free(acc->password);
    free(acc->out_file);
    free(acc);
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
        if(strcmp(accountArray[i]->account_number, accountID) == 0)
            return accountArray[i];
    } 
    return NULL;
}


void PrintBalances(account **account_array, int numacs)
{
    int i;

    for (i = 0; i<numacs; i++)
        printf("%d balance:\t%.2f\n", i, account_array[i]->balance);
    return;
}

