#ifndef PARSER_H
#define PARSER_H

typedef struct cmd {
    char **argv;
    int size;
} cmd;

cmd *ParseLine(char *line, const char *delim);

int CountTokens(char *buf, const char *delim);

void FreeCmd(cmd *command);

#endif