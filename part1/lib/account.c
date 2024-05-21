#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "list.h"


account *initacc(char *id, char *pass, char *outfile, double balance,
    double reward_rate, double transaction_tracker)
{
    account *newacc;
    newacc = (account *)malloc(sizeof(account));
    newacc->account_number = strdup(id);
    newacc->password = strdup(pass);
    newacc->out_file = strdup(outfile);
    newacc->balance = balance;
    newacc->reward_rate = reward_rate;
    newacc->transaction_tracker = transaction_tracker;
    if (pthread_mutex_init(&(newacc->ac_lock), NULL) != 0) {
        free(newacc); // Free memory if mutex initialization fails
        return NULL;
    }
    return newacc;
}


void freeacc(account *acc)
{
    free(acc->account_number);
    free(acc->password);
    free(acc->out_file);
    free(acc);
}


account *find(list *acclist, char *accnum)
{
    account *ac;
    node *cnode = NULL;
    while ((ac=(account *)inorder(acclist, &cnode))!= NULL) {
        if (strcmp(ac->account_number, accnum) == 0) {
            return ac;
        }
    }
    return ac;
}