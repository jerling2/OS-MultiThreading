#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "account.h"
#include "request.h"


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
