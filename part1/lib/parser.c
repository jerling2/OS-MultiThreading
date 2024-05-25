/*
Joseph Erlinger (jerling2@uoregon.edu)
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"


/**
 * @brief Create a cmd structure from a input line buffer.
 * 
 * This function uses strtok_r to create a cmd structure that contains tokens
 * from an input line buffer. The input line buffer will not be altered.
 * 
 * @param[in] line Pointer to a buffer.
 * @param[in] delimiter to split the line buffer into tokens.
 * @return cmd structure containing tokens parsed from the line buffer.
*/
cmd *parseline (char *line, const char *delim)
{
    cmd *command;     // Cmd structure to contain the processed input line.
    char *linedup;    // Duplicate string to preserve the original line.
    char *saveptr;    // (Use internally by strtok_r)
    char *token;      // Pointer to a token returned by strtok_r.
    int i;            // The ith token in the line.

    i = 0;
    linedup = strdup(line);
    command = (cmd*)malloc(sizeof(cmd));
    command->size = numtok(linedup, delim);
    command->argv = (char**)malloc(sizeof(char*)*command->size);
	token = strtok_r(linedup, delim, &saveptr);         // Get the first token.
	while (token != NULL) {
		command->argv[i] = strdup(token);
		token = strtok_r(NULL, delim, &saveptr);
		i ++;
	}
    command->argv[i] = NULL;             // Last argument in argv must be null.
    free(linedup);
    return command;
}   /* parseline */


/**
 * @brief Count the number of tokens in a buffer.
 * 
 * This function uses a finite state machine pattern to count the number of
 * tokens in a buffer based on a input delimiter.
 * 
 * @caution This function has a chance to modify the input buffer.
 * 
 * @param[in,out] buffer Pointer that will be read from. (Note Caution)
 * @param[in] delimeter to distinguish the start/end of a token.
 * @return integer count of the number of tokens in the buffer.
 */
int numtok (char *buf, const char *delim)
{
    int tok;      // Number of tokens.
    int state;    // The current state of the algorithm.
    int i;        // The ith character in the buf.

    tok = 1;                    // Start at 1 to make space for the null token.
    state = 0;
    i = 0;
    strtok (buf, "\n");                    // Strip newline character (if any).
    /* Skip all leading delimiters */
    while (buf[i] == delim[0]) {
        i ++;
    }	
    /* Increment tokens only when moving from state 0 to 1 */
    for (; buf[i] != '\0'; i++) {  
        /* State 0: Skip all delimiters */
        if (state == 0 && buf[i] != delim[0]) { // 
            state = 1;
            tok ++;                                       // Increment tokens.
        }
        /* State 1: Skip all non-delimiters */
        if (state == 1 && buf[i] == delim[0]) { //  
            state = 0;
        }
    }
    return tok;
}   /* numtok */


/**
 * @brief Deallocate a cmd structure.
 * 
 * This function releases all the memory reserved by a command structure.
 * 
 * @param[in] command Pointer to be deallocated.
*/
void freecmd (cmd *command)
{
    int i;    // The ith argument in command's argv.

    for (i = 0; i < command->size; i++) {
        free(command->argv[i]);
    }
    free(command->argv);
    free(command);
}   /* freecmd */
