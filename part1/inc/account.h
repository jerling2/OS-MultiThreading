#ifndef ACCOUNT_H
#define ACCOUNT_H
#include <stdio.h>  // For FILE struct
#include <pthread.h>

typedef struct account {
	char *accountNumber; // 17 bytes
	char *password; // 9 bytes
    char *outFile; // 64 bytes
    double balance;
    double rewardRate;
    double transactionTracker;
    pthread_mutex_t lock;
    int priority;
} account;

account *InitAccount(char *id, char *pass, char *outfile, double balance, 
    double reward_rate, int order);

void FreeAccount(account *acc);

void FreeAccountArray(account **accountArray, int arraySize);

account *Find(account **accountArray, char *accountID, int arraySize);

void PrintBalances(FILE *stream, account **accountArray, int totalAccounts);

#endif /* ACCOUNT_H */