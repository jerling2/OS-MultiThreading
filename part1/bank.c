#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "account.h"
#include "fileio.h"
#include "list.h"
#include "parser.h"
#include "request.h"
#include "config.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define WARNING "\x1b[1;31mWARNING\x1b[0m"


int main (int argc, char *argv[]) 
{
    FILE *stream;
    char *filename;
    hashmap *account_hashmap;
    account **account_array;
    int numacs;
    cmd *request;
    int i;

    if (argc != 2) {
        printf("%s usuage %s <filename>\n", ERROR, argv[0]);
        exit(EXIT_FAILURE);
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {
        printf("%s %s\n", ERROR, strerror(errno));
        exit(EXIT_FAILURE);
    }
    account_hashmap = getAccounts(stream, filename, &account_array, &numacs);
    if (account_hashmap == NULL) {
        fclose(stream);
        exit(EXIT_FAILURE);   
    }
    i = numacs * 5 + 2;  // total accounts * lines per account init + maxindex + one-index.
    while ((request = readRequest(stream)) != NULL) {
        if (commandInterpreter(account_hashmap, request) == -1) {
            printf("%s:%d\n", filename, i);
        }
        freecmd(request);
        i ++;
    }
    for (i = 0; i<numacs; i++) {
        printf("%d balance:\t%.2f\n", i, account_array[i]->balance + account_array[i]->transaction_tracker * account_array[i]->reward_rate);
    }
    freeHashmap(account_hashmap, (void *)freeacc);
    free(account_array);
    fclose(stream);
}