#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <pthread.h>
#include "list.h"

typedef struct account {
	char *account_number; // 17 bytes
	char *password; // 9 bytes
    char *out_file; // 64 bytes
    double balance;
    double reward_rate;
    double transaction_tracker;
    pthread_mutex_t ac_lock;
} account;

account *initacc(char *id, char *pass, char *outfile, double balance,
    double reward_rate);

void freeacc(account *acc);

account *find(hashmap *account_hm, char *account_id);

void print_balances(account **account_array, int numacs);

#endif /* ACCOUNT_H */