/*
Joseph Erlinger (jerling2@uoregon.edu)

This is part1 of the project and serves as the main driver for completing the
task outlined by the project description.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "account.h"
#include "fileio.h"
#include "parser.h"
#include "request.h"
#define _GNU_SOURCE

// Macro for turning on/off debug messages.
#ifdef DEBUG_ENABLED
#define DEBUG if (1)
#else
#define DEBUG if (0)
#endif

// Macro for turning on/off info messages.
#ifdef INFO_ENABLED
#define INFO if (1)
#else
#define INFO if (0)
#endif

// For notifying invalid usuage.
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define USUAGE "%s usuage %s <filename>\n"
#define STREAM "%s could not open '%s'. %s.\n"


/**
 * @brief The main thread (Single threaded solution)
 * 
 * This function is run by the main thread. The main thread opens the input
 * file, extracts the accounting information, processes each request, and
 * then exits. This is the single threaded solution, where every instruction
 * is completed sequentially.
 */
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
    DEBUG printf("-- Part 1 --\n");
    DEBUG printf("main (pid=%u) is extracting the accounting information...", getpid());
    GetAccounts(stream, &accountArray, &totalAccounts);
    WriteFileHeader(accountArray, totalAccounts, 1);
    DEBUG printf("DONE\n");
    /* Process requests from the input file */
    DEBUG printf("main is processing requests...");
    while ((request = ReadRequest(stream)) != NULL) {
        CommandInterpreter(accountArray, request, totalAccounts);
        FreeCmd(request);
    }
    DEBUG printf("DONE\n");
    /* Apply the reward rate to all balances */
    DEBUG printf("main is adding each account's reward to their balance...");
    ProcessReward(accountArray, totalAccounts);
    DEBUG printf("DONE\n");
    /* Output data to standard out */
    INFO printf("Duck Bank Balances:\n");
    INFO PrintBalances(stdout, accountArray, totalAccounts);
    /* Create "output/output.txt" */
    WriteFinalBalances(accountArray, totalAccounts);
    /* Free resources */
    FreeAccountArray(accountArray, totalAccounts);
    fclose(stream);
}