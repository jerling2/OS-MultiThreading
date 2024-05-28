#ifndef REQUEST_H
#define REQUEST_H
#include "list.h"
#include "account.h"
#include "parser.h"

int commandInterpreter(hashmap *hm, cmd *command);

int verify_password(account *acc, char *pass);

void transfer(account *acc1, account *acc2, double funds);

void deposit(account *acc, double funds);

void update_tracker(account *acc, double funds);

void withdraw(account *acc, double funds);

void check(account *acc);

void process_reward(account **account_array, int numacs);

#endif