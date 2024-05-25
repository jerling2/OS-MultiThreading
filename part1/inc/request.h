#ifndef REQUEST_H
#define REQUEST_H
#include "list.h"
#include "account.h"

void transfer(account *acc1, account *acc2, double funds);
void deposit(account *acc, double funds);
void withdraw(account *acc, double funds);
void check(account *acc);

#endif