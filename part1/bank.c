#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "account.h"
#include "fileio.h"
#include "list.h"
#include "parser.h"
#include "request.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"


int main (int argc, char *argv[]) 
{
    FILE *stream;
    char *filename;
    hashmap *account_hashmap;
    account **account_array;
    int numacs;
    cmd *request;

    /* Validate input. */
    if (argc != 2) {
        fprintf(stderr, USUAGE, ERROR, argv[0]);
        exit(EXIT_FAILURE); 
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {
        printf(STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Extract account data and Create datastructures */
    getAccounts(stream, filename, &account_hashmap, &account_array, &numacs);
    /* Process requests from the input file */
    while ((request = readRequest(stream)) != NULL) {
        CommandInterpreter(account_hashmap, request);
        freecmd(request);
    }
    /* Apply the reward rate to all balances */
    ProcessReward(account_array, numacs);
    /* Output data to standard out */
    print_balances(account_array, numacs);
    /* Free resources */
    freeHashmap(account_hashmap, (void *)freeacc);
    free(account_array);
    fclose(stream);
}