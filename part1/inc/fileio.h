#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "parser.h"
#include "account.h"

void GetAccounts(FILE *stream, char *filename, account ***acs, int *numac);

cmd *ReadRequest(FILE *stream);

int GetFromPattern(FILE *stream, char *pattern, void *data);

#endif /* FILEIO_H */