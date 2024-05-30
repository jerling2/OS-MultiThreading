#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "list.h"
#include "account.h"
#include "parser.h"
#include "request.h"
#include "thread.h"


#ifdef SINGLE_THREAD
int commandInterpreter(hashmap *hm, cmd *command)
{
    char *op;
    char **argv;
    int numarg;
    account *a1;
    account *a2;
    double funds;

    op = command->argv[0];
    argv = command->argv;
    numarg = command->size - 2;
    if (op == NULL || numarg < 2)
        return -1;                          // ERROR: Incorrect request syntax.
    if ((a1 = find(hm, argv[1])) == NULL)
        return -1;                          // ERROR: Could not find account 1.
    if (verify_password(a1, argv[2]) == -1) {
        return 1;
    }
    if (strcmp(op, "T") == 0 && numarg == 4) {
        if ((a2 = find(hm, argv[3])) == NULL)
            return -1;                      // ERROR: Could not find account 2.
        funds = strtod(argv[4], NULL);
        transfer(a1, a2, funds);
        update_tracker(a1, funds);
    } else 
    if (strcmp(op, "W") == 0 && numarg == 3) {
        funds = strtod(argv[3], NULL);
        withdraw(a1, funds);
        update_tracker(a1, funds);
    } else
    if (strcmp(op, "D") == 0 && numarg == 3) {
        funds = strtod(argv[3], NULL);
        deposit(a1, funds);
        update_tracker(a1, funds);
    } else
    if (strcmp(op, "C") == 0 && numarg == 2) {
        check(a1);
    } else {
        return -1;                              // ERROR: Unrecognized request.
    }
    return 1;
}
#elif defined(MULTI_THREAD)
int commandInterpreter(hashmap *hm, cmd *command, requestCounter *rc)
{
    char *op;
    char **argv;
    account *a1, *a2;
    int numarg, numacs;
    double funds;

    op = command->argv[0];
    argv = command->argv;
    numarg = command->size - 2;
    if (op == NULL || numarg < 2)
        return -1;                          // ERROR: Incorrect request syntax.
    if ((a1 = find(hm, argv[1])) == NULL)
        return -1;                          // ERROR: Could not find account 1.
    if (verify_password(a1, argv[2]) == -1) {
        return 1;
    }
    if (numarg==4) {
        a2 = find(hm, argv[3]);
        numacs = 2;
    } else {
        a2 = NULL;
        numacs = 1;
    }
    obtain_locks(numacs, a1, a2);
    pthread_mutex_lock(&rc->rc_lock);
    if (strcmp(op, "T") == 0) {
        funds = strtod(argv[4], NULL);
        transfer(a1, a2, funds);
        update_tracker(a1, funds);
        incrementCount(rc);
    } else
    if (strcmp(op, "W") == 0) {
        funds = strtod(argv[3], NULL);
        withdraw(a1, funds);
        update_tracker(a1, funds);
        incrementCount(rc);
    } else
    if (strcmp(op, "D") == 0) {
        funds = strtod(argv[3], NULL);
        deposit(a1, funds);
        update_tracker(a1, funds);
        incrementCount(rc);
    } else
    if (strcmp(op, "C") == 0) {
        check(a1);
    } 
    pthread_mutex_unlock(&rc->rc_lock);
    release_locks(numacs, a1, a2);
    return 1;
}
#endif

int verify_password(account *acc, char *pass)
{
    if (strcmp(acc->password, pass) != 0)
        return -1;
    return 0;
}


void transfer(account *acc1, account *acc2, double funds)
{
    withdraw(acc1, funds);
    deposit(acc2, funds);
}


void withdraw(account *acc, double funds)
{
    acc->balance -= funds;
}


void deposit(account *acc, double funds)
{
    acc->balance += funds;
}


void update_tracker(account *acc, double funds)
{
    acc->transaction_tracker += funds;
}


void process_reward(account **account_array, int numacs)
{
    double reward;
    int i;

    for (i = 0; i<numacs; i++) {
        reward = account_array[i]->transaction_tracker;
        reward *= account_array[i]->reward_rate;
        account_array[i]->balance += reward;
        account_array[i]->transaction_tracker = 0;
    }
}


void check(account *acc)
{
    FILE *stream;

    stream = fopen(acc->out_file, "a");
    fprintf(stream, "balance: %.2f\n", acc->balance);
    fflush(stream);
    fclose(stream);
}


void incrementCount(requestCounter *rc)
{
    pthread_mutex_lock(&worker_bank_sync.sync_lock);
    rc->count ++;
    if (rc->count == 5000) {
        pthread_cond_signal(&worker_bank_sync.cond1);
        pthread_cond_wait(&worker_bank_sync.cond2, &worker_bank_sync.sync_lock);
        rc->count = 0;
    }
    pthread_mutex_unlock(&worker_bank_sync.sync_lock);
}
