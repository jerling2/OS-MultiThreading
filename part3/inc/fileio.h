#ifndef FILEIO_H
#define FILEIO_H
#include <stdio.h>
#include "parser.h"
#include "account.h"

void GetAccounts(FILE *stream, account ***accountArray, int *totalAccounts);

cmd *ReadRequest(FILE *stream);

void GetFromPattern(FILE *stream, char *pattern, void *data);

void WriteOutput(char *filename, account **accountArray, int totalAccounts);

// Wrapper function for WriteOutput
void WriteFinalBalances(account **accountArray, int totalAccounts);

// Wrapper function for WriteOutput
void WriteFinalSavings(account **accountArray, int totalAccounts);

#endif /* FILEIO_H */