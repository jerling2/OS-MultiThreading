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
#define WARNING "\x1b[1;31mWARNING\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"
#define REQUEST "%s invalid request %s:%d.\n"


int main (int argc, char *argv[]) 
{
    FILE *stream;
    char *filename;
    hashmap *account_hashmap;
    account **account_array;
    int numacs;
    cmd *request;
    int line_number;

    if (argc != 2) {                                         // Validate Input.
        fprintf(stderr, USUAGE, ERROR, argv[0]);
        exit(EXIT_FAILURE); 
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {              // Open stream.
        printf(STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    getAccounts(stream, filename, &account_hashmap, &account_array, &numacs);
    if (numacs == 0) {                              // Handle getAccount Error.
        fclose(stream);
        exit(EXIT_FAILURE);   
    }
    line_number = numacs * 5 + 2;                     // Line number for DEBUG.
    while ((request = readRequest(stream)) != NULL) {
        if (commandInterpreter(account_hashmap, request) == -1)
            fprintf(stderr, REQUEST, WARNING, filename, line_number); // DEBUG.
        freecmd(request);
        line_number ++;
    }
    // process_reward(account_array, numacs);
    print_balances(account_array, numacs);
    freeHashmap(account_hashmap, (void *)freeacc);
    free(account_array);
    fclose(stream);
}