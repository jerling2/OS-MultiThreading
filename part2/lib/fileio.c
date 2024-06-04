#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fileio.h"
#include "account.h"
#include "parser.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define LOGFILE "log/account_%d.log"       //< IMPORTANT: directory must exist.


// returns a list of accounts. 
void GetAccounts(FILE *stream, char *filename, account ***accountArray, int *totalAccounts)
{
    account *newAccount;          // A pointer to an account.
    char accountNumber[17];       // Temp buffer to hold the account number.
    char password[9];             // Temp buffer to hold the accound password.
    char outfile[64];             // Temp buffer to hold the outfile path.
    double balance;               // Account balance.
    double rewardRate;            // Account reward rate.
    int maxIndex = -1;            // Max number of accounts.
    int currentIndex = -1;        // Current account.
    int i = 1;                    // Line number.
    int k = 0;                    // Next expected index number.

    (*totalAccounts) = 0;
    (*accountArray) = NULL;
    if (GetFromPattern(stream, "%d", &maxIndex) == -1) {
        fprintf(stderr, "%s %s %s:%d\n", 
        ERROR, "missing total # accounts (int)", filename, i);
        goto error;
    }
    (*accountArray) = (account **)malloc(sizeof(account *)*maxIndex);
    i ++;
    while (currentIndex + 1 < maxIndex) {
        if (GetFromPattern(stream, "index %d", &currentIndex) == -1 || currentIndex != k) {
            fprintf(stderr, "%s %s %d' in %s:%d\n", 
            ERROR, "missing 'index", k, filename, i);
            goto error;
        }
        if (GetFromPattern(stream, "%17s", accountNumber) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account number (char *)", filename, i+1);
            goto error;
        }
        if (GetFromPattern(stream, "%9s", password) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account password (char *)", filename, i+2);
            goto error;
        }
        if (GetFromPattern(stream, "%lf", &balance) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account balance (double)", filename, i+3);
            goto error;
        }
        if (GetFromPattern(stream, "%lf", &rewardRate) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account reward rate (double)", filename, i+4);
            goto error;
        }
        snprintf(outfile, 64, LOGFILE, currentIndex);
        newAccount = InitAccount(accountNumber, password, outfile, balance, rewardRate, k);
        (*accountArray)[currentIndex] = newAccount;
        (*totalAccounts)++;
        i += 5;
        k ++;
    }
    return;

    error:
    // freeHashmap((*account_hashmap), (void *)freeacc);
    free(*accountArray);
    (*totalAccounts) = 0;
    return;
}


cmd *ReadRequest (FILE *stream)
{
    char line[BUFSIZ];    // The line read from a file stream.
    cmd *command;

    command = NULL;
    if (fgets(line, BUFSIZ, stream) != NULL) {
        command = ParseLine(line, " ");                               
    }
    if (errno != 0) {              
        fprintf(stderr, "%s readcmd(): %s\n", ERROR, strerror(errno));
    }
    return command;
}


int GetFromPattern(FILE *stream, char *pattern, void *data)
{
    char line[BUFSIZ];
    if (fgets(line, BUFSIZ, stream) == NULL)
        return -1;
    if (sscanf(line, pattern, data) <= 0)
        return -1;
    return 1;
}