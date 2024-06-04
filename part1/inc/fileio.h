#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "parser.h"
#include "account.h"


void getAccounts(FILE *stream, char *filename, account ***acs, int *numac);
// void getAccounts(FILE *stream, char *filename, 
//     hashmap **account_hashmap, account ***acs, int *numac);

cmd *readRequest(FILE *stream);

int extractitem(FILE *stream, char *pattern, void *data);

#endif /* FILEIO_H */