#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "account.h"
#include "parser.h"
#include "request.h"


int commandInterpreter(hashmap *hm, cmd *command)
{
    char *op;
    char **argv;
    int numarg;
    account *a1;
    account *a2;

    op = command->argv[0];
    argv = command->argv;
    numarg = command->size - 2;
    if (op == NULL || numarg < 2)
        return -1;
    if ((a1 = find(hm, argv[1])) == NULL)
        return -1;
    if (verify_password(a1, argv[2]) == -1)
        return -1;
    if (strcmp(op, "T") == 0 && numarg == 4) {
        if ((a2 = find(hm, argv[3])) == NULL)
            return -1;
        transfer(a1, a2, strtod(argv[4], NULL));
    } else 
    if (strcmp(op, "W") == 0 && numarg == 3) {
        withdraw(a1, strtod(argv[3], NULL));
    } else
    if (strcmp(op, "D") == 0 && numarg == 3) {
        deposit(a1, strtod(argv[3], NULL));
    } else
    if (strcmp(op, "C") == 0 && numarg == 2) {
        check(a1);
    } else {
        return -1;
    }
    return 1;
}


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
    // Add #funds to acc
    acc->balance -= funds;
}


void deposit(account *acc, double funds)
{
    acc->balance += funds;
}


void check(account *acc)
{
    FILE *stream;

    stream = fopen(acc->out_file, "w");
    fprintf(stream, "balance: %.2f\n", acc->balance);
    fflush(stream);
    fclose(stream);
}
