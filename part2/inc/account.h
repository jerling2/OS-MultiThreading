#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <pthread.h>

typedef struct account {
	char *account_number; // 17 bytes
	char *password; // 9 bytes
    char *out_file; // 64 bytes
    double balance;
    double reward_rate;
    double transaction_tracker;
    pthread_mutex_t ac_lock;
    int order;
} account;

account *initacc(char *id, char *pass, char *outfile, double balance, 
    double reward_rate, int order);

void freeacc(account *acc);

void FreeAccountArray(account **accountArray, int arraySize);

account *find(account **accountArray, char *accountID, int arraySize);

void print_balances(account **account_array, int numacs);

#endif /* ACCOUNT_H */