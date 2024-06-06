/*
Joseph Erlinger (jerling2@uoregon.edu)

This file "file.c" is responsible for reading account and request data from an
input file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "fileio.h"
#include "account.h"
#include "parser.h"
#define ERROR "\x1b[1;31mERROR\x1b[0m"
#define LOGFILE "log/account_%d.log"       //< IMPORTANT: directory must exist.


/**
 * @brief Initialize an accountArray with the account data read from an file.
 * 
 * This function processes the account information from an input file into a
 * array of account structures. The account information should be the first
 * lines of the input file. The input file is assumed to have no errors, as
 * stated by the project description. As such, this function does not do any
 * error handling. 
 * 
 * Note: This function does not open or close the FILE *. When this function
 * returns, the FILE * will be pointing to the first request of the input file.
 * 
 * @param[in,out ] stream (FILE *) A path to the input file.
 * @param[in,out] accountArray (account ***) An uninitialized accountArray.
 * @param[in,out] totalAccounts (int *) An uninitialized integer.
 * @return An initialized accountArray pointer and totalAccounts pointer.
 */
void GetAccounts(FILE *stream, account ***accountArray, int *totalAccounts)
{
    account *newAccount;          // A pointer to an account.
    char accountNumber[17];       // Temp buffer to hold the account number.
    char password[9];             // Temp buffer to hold the accound password.
    char outfile[64];             // Temp buffer to hold the outfile path.
    double balance;               // Account balance.
    double rewardRate;            // Account reward rate.
    int maxIndex;                 // Max number of accounts.
    int currentIndex;             // Current account.

    (*totalAccounts) = 0;
    (*accountArray) = NULL;
    GetFromPattern(stream, "%d", &maxIndex);           // Initializes maxIndex.
    (*accountArray) = (account **)malloc(sizeof(account *)*maxIndex);
    currentIndex = 0;
    while (currentIndex + 1 < maxIndex) {
        GetFromPattern(stream, "index %d", &currentIndex);
        GetFromPattern(stream, "%17s", accountNumber);
        GetFromPattern(stream, "%9s", password);
        GetFromPattern(stream, "%lf", &balance);
        GetFromPattern(stream, "%lf", &rewardRate);
        snprintf(outfile, 64, LOGFILE, currentIndex);
        newAccount = InitAccount(accountNumber, password, outfile, balance, rewardRate, currentIndex);
        (*accountArray)[currentIndex] = newAccount;
        (*totalAccounts)++;
    }
}


/**
 * @brief Parse a request from the input file into a cmd structure.
 * 
 * This function parses a line from an input file that is pointed to by the
 * stream pointer into a cmd structure. The line that is parsed should
 * represent a bank request.
 * 
 * Note: All requests are assumed to be correctly formed as outlined by the 
 * project description. As such, there is no error handling in this function.
 * 
 * @param[in,out] stream (FILE *) A file pointer that points to a 'request' line.
 * @return An cmd structure representing a bank request.
 */
cmd *ReadRequest (FILE *stream)
{
    char line[BUFSIZ];    // The line read from the input file.
    cmd *command;         // The cmd structure representing a bank request.

    command = NULL;
    if (fgets(line, BUFSIZ, stream) != NULL)
        command = ParseLine(line, " ");                   
    return command;
}


/**
 * @brief Initialize some data by scanning it from a file. 
 * 
 * This function uses fgets and sscanf to retrieve the next line of a file
 * and initialize data by scanning the line with a pattern. GetAccounts calls 
 * this function to translate file data into account struct data.
 * 
 * Note: All account lines are assumed to be correct as outlined by the project
 * description. As such, there is no error handling done by this function.
 * 
 * @param[in,out] stream (FILE *) A file pointer that points to a line that is encoding 
 *                            some account information.
 * @param[in] pattern (char *) A pattern used by sscanf.
 * @param[in,out] data (void *) Some data to be initialized by sscanf.
 */
void GetFromPattern(FILE *stream, char *pattern, void *data)
{
    char line[BUFSIZ];

    fgets(line, BUFSIZ, stream);
    sscanf(line, pattern, data);
}