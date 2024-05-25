#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "list.h"
#include "parser.h"

hashmap *getAccounts(FILE *stream, char *filename);
cmd *readRequest(FILE *stream);
int extractitem(FILE *stream, char *pattern, void *data);
#endif /* FILEIO_H */