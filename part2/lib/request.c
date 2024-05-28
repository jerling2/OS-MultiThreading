#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "account.h"
#include "parser.h"
#include "request.h"

#define MULTI_THREAD

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
        // START: LOCK 2
        // START: LOCK 1
        transfer(a1, a2, funds);
        update_tracker(a1, funds);
        // END: LOCK 2
        // END: LOCK 1
    } else 
    if (strcmp(op, "W") == 0 && numarg == 3) {
        funds = strtod(argv[3], NULL);
        // START: LOCK 1
        withdraw(a1, funds);
        update_tracker(a1, funds);
        // END: LOCK 1
    } else
    if (strcmp(op, "D") == 0 && numarg == 3) {
        funds = strtod(argv[3], NULL);
        // START: LOCK 1
        deposit(a1, funds);
        update_tracker(a1, funds);
        // END: LOCK 2
    } else
    if (strcmp(op, "C") == 0 && numarg == 2) {
        // START: LOCK 1
        check(a1);
        // END: LOCK 1
    } else {
        return -1;                              // ERROR: Unrecognized request.
    }
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