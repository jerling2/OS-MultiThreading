#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "account.h"
#include "list.h"


account *initacc(char *id, char *pass, char *outfile, double balance, 
    double reward_rate)
{
    FILE *logfile;
    account *newacc;

    newacc = (account *)malloc(sizeof(account));
    newacc->account_number = strdup(id);
    newacc->password = strdup(pass);
    newacc->out_file = strdup(outfile);
    newacc->balance = balance;
    newacc->reward_rate = reward_rate;
    newacc->transaction_tracker = 0;
    if (pthread_mutex_init(&(newacc->ac_lock), NULL) != 0) {
        free(newacc); // Free memory if mutex initialization fails
        return NULL;
    }
    logfile = fopen(outfile, "w");
    fclose(logfile);
    return newacc;
}


void freeacc(account *acc)
{
    free(acc->account_number);
    free(acc->password);
    free(acc->out_file);
    free(acc);
}


account *find(hashmap *account_hm, char *account_id)
{
    unsigned long hashvalue;
    int index;
    list *l;
    node *cnode;
    account *ac;
    
    cnode = NULL;
    hashvalue = hash((unsigned char *) account_id);
    index = hashvalue % account_hm->size;
    if ((l = account_hm->map[index]) == NULL)
        return NULL;
    while ((ac=(account *)inorder(l, &cnode))!= NULL) {
        if (strcmp(ac->account_number, account_id) == 0) {
            return ac;
        }
    }
    return NULL;
}


void print_balances(account **account_array, int numacs)
{
    int i;

    for (i = 0; i<numacs; i++)
        printf("%d balance:\t%.2f\n", i, account_array[i]->balance);
    return;
}

