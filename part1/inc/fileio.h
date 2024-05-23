#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "list.h"

hashmap *getAccounts(char *filename);
int extractitem(FILE *stream, char *pattern, void *data);
#endif /* FILEIO_H */