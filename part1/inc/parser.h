#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>

typedef struct cmd {
    char **argv;
    int size;
} cmd;

cmd *parseline (char *line, const char *delim);

int numtok (char *buf, const char *delim);

void freecmd (cmd *command);

#endif