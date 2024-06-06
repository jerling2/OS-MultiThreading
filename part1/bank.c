#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"
#define _GNU_SOURCE
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"


int main (int argc, char *argv[]) 
{
    FILE *stream;
    char *filename;
    account **accountArray;
    int totalAccounts;
    cmd *request;

    /* Validate input */
    if (argc != 2) {
        fprintf(stderr, USUAGE, ERROR, argv[0]);
        exit(EXIT_FAILURE); 
    }
    filename = argv[1];
    if ((stream = fopen(filename, "r")) == NULL) {
        printf(STREAM, ERROR, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Extract account data */
    GetAccounts(stream, &accountArray, &totalAccounts);
    /* Process requests from the input file */
    while ((request = ReadRequest(stream)) != NULL) {
        CommandInterpreter(accountArray, request, totalAccounts);
        FreeCmd(request);
    }
    /* Apply the reward rate to all balances */
    ProcessReward(accountArray, totalAccounts);
    /* Output data to standard out */
    PrintBalances(accountArray, totalAccounts);
    /* Free resources */
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}