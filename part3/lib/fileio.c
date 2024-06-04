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
void getAccounts(FILE *stream, char *filename, account ***acs, int *numac)
{
    account *ac;                  // A pointer to an account.
    char acnumber[17];            // Temp buffer to hold the account number.
    char password[9];             // Temp buffer to hold the accound password.
    char outfile[64];             // Temp buffer to hold the outfile path.
    double balance;               // Account balance.
    double rewardrate;            // Account reward rate.
    int maxindex = -1;            // Max number of accounts.
    int cindex = -1;              // Current account.
    int i = 1;                    // Line number.
    int k = 0;                    // Next expected index number.

    (*numac) = 0;
    // (*account_hashmap) = NULL;
    (*acs) = NULL;
    if (extractitem(stream, "%d", &maxindex) == -1) {
        fprintf(stderr, "%s %s %s:%d\n", 
        ERROR, "missing total # accounts (int)", filename, i);
        goto error;
    }
    (*acs) = (account **)malloc(sizeof(account *)*maxindex);
    // hashmap_size = nextPowerOf2(maxindex) * 2;
    // (*account_hashmap) = initHashmap(hashmap_size);
    i ++;
    while (cindex + 1 < maxindex) {
        if (extractitem(stream, "index %d", &cindex) == -1 || cindex != k) {
            fprintf(stderr, "%s %s %d' in %s:%d\n", 
            ERROR, "missing 'index", k, filename, i);
            goto error;
        }
        if (extractitem(stream, "%17s", acnumber) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account number (char *)", filename, i+1);
            goto error;
        }
        if (extractitem(stream, "%9s", password) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account password (char *)", filename, i+2);
            goto error;
        }
        if (extractitem(stream, "%lf", &balance) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account balance (double)", filename, i+3);
            goto error;
        }
        if (extractitem(stream, "%lf", &rewardrate) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            ERROR, "missing account reward rate (double)", filename, i+4);
            goto error;
        }
        snprintf(outfile, 64, LOGFILE, cindex);
        ac = initacc(acnumber, password, outfile, balance, rewardrate, k);
        // insert((*account_hashmap), acnumber, ac);
        (*acs)[cindex] = ac;
        (*numac)++;
        i += 5;
        k ++;
    }
    return;

    error:
    // freeHashmap((*account_hashmap), (void *)freeacc);
    free(*acs);
    (*numac) = 0;
    return;
}


cmd *readRequest (FILE *stream)
{
    char line[BUFSIZ];    // The line read from a file stream.
    cmd *command;

    command = NULL;
    if (fgets(line, BUFSIZ, stream) != NULL) {
        command = parseline(line, " ");                               
    }
    if (errno != 0) {              
        fprintf(stderr, "%s readcmd(): %s\n", ERROR, strerror(errno));
    }
    return command;
}


int extractitem(FILE *stream, char *pattern, void *data)
{
    char line[BUFSIZ];
    if (fgets(line, BUFSIZ, stream) == NULL)
        return -1;
    if (sscanf(line, pattern, data) <= 0)
        return -1;
    return 1;
}