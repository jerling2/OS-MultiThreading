#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "parser.h"
#include "account.h"

#define LOGFILE "log/account_%d.log"  

void GetAccounts(FILE *stream, account ***accountArray, int *totalAccounts);

cmd *ReadRequest(FILE *stream);

void GetFromPattern(FILE *stream, char *pattern, void *data);

#endif /* FILEIO_H */