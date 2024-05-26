#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "list.h"
#include "parser.h"
#include "account.h"

hashmap *getAccounts(FILE *stream, char *filename, account ***acs, int *numac);

cmd *readRequest(FILE *stream);

int extractitem(FILE *stream, char *pattern, void *data);

#endif /* FILEIO_H */