#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"
#include "account.h"
#include "list.h"
#define CRITICAL "\x1b[1;31mCritical\x1b[0m"


int getAccounts(char *filename)
{
    FILE *stream;
    int maxindex = -1;
    int cindex = -1;
    int i = 1;
    int k = 0;
    char line[BUFSIZ];

    char acnumber[17];
    char password[9];
    char outfile[64];
    double balance;
    double rewardrate;

    if ((stream = fopen(filename, "r")) == NULL) {
        perror("\x1b[1;31mCritical\x1b[0m fopen()");
        return -1;
    }
    if (extractitem(stream, "%d", &maxindex) == -1) {
        fprintf(stderr, "%s %s in %s:%d\n", 
        CRITICAL, "expecting total # accounts (int)", filename, i);
        goto error;
    }
    i ++;
    while (fgets(line, BUFSIZ, stream) != NULL && cindex + 1 < maxindex) {
        sscanf(line, "index %d", &cindex);
        if (cindex != k) {
            fprintf(stderr, "%s %s %d' in %s:%d\n", 
            CRITICAL, "expecting 'index", k, filename, i);
            goto error;
        }
        if (extractitem(stream, "%17s", acnumber) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            CRITICAL, "expecting account number (char *)", filename, i+1);
            goto error;
        }
        if (extractitem(stream, "%9s", password) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            CRITICAL, "expecting account password (char *)", filename, i+2);
            goto error;
        }
        if (extractitem(stream, "%lf", &balance) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            CRITICAL, "expecting account balance (double)", filename, i+3);
            goto error;
        }
        if (extractitem(stream, "%lf", &rewardrate) == -1) {
            fprintf(stderr, "%s %s in %s:%d\n", 
            CRITICAL, "expecting account reward rate (double)", filename, i+4);
            goto error;
        }
        snprintf(outfile, 64, "account_%d_%s.log", cindex, acnumber);
        initacc(acnumber, password, outfile, balance, rewardrate);
        i += 5;
        k ++;
    }
    fclose(stream);
    return 1;

    error:
    fclose(stream);
    // free(acclist);
    return -1;
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