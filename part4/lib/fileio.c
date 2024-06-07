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
#define LOGFILE "output/account%d.txt"
#define OUT_BALANCES_FILE "output/output.txt"
#define OUT_SAVINGS_FILE "savings/output.txt"


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


/**
 * @brief Append an account's balance to the accounts outFile.
 * 
 * ProcessReward and UpdateSavings in request.c calls this function after an
 * account receives its reward.
 * 
 * @param[in] account (account *) Account.
 * @param[in] flag (int) used to indicate either Duck Bank or Puddles Bank.
 * @return balance data appended to the account's outFile.
 */
void AppendToFile(account *account, int flag)
{
    FILE *stream;    // The stream of the account's outFile.

    stream = fopen(account->outFile, "a");
    /* We have to match the expected files EXACTLY */
    if (flag) fprintf(stream, "Current Balance:\t%.2f\n", account->balance);
    else      fprintf(stream, "Current Savings Balance  %.2f\n", account->balance);
    fflush(stream);
    fclose(stream);
}


/**
 * @brief Write final account balances to an output file.
 * 
 * @param[in] filename (char *) The output file.
 * @param[in] accountArray (account **) The account balances to be printed.
 * @param[in] totalAccounts (int) the size of accountArray.
 */
void WriteOutput(char *filename, account **accountArray, int totalAccounts)
{
    FILE *stream;

    stream = fopen(filename, "w");
    if (stream == NULL) {
        perror("fopen");
        return;
    }
    PrintBalances(stream, accountArray, totalAccounts);
    fclose(stream);
}


// Wrapper for WriteOutput
void WriteFinalBalances(account **accountArray, int totalAccounts)
{
    WriteOutput(OUT_BALANCES_FILE, accountArray, totalAccounts);
}

// Wrapper for WriteOutput
void WriteFinalSavings(account **accountArray, int totalAccounts)
{
    WriteOutput(OUT_SAVINGS_FILE, accountArray, totalAccounts);
}


// This function is used to match the expected output on Canvas.
// The styles are inconsistent so this function has a flag to switch between
// the two different ':' placements.
void WriteFileHeader(account **accountArray, int totalAccounts, int flag)
{
    FILE *stream;
    int i;

    for (i=0; i<totalAccounts; i++) {
        stream = fopen(accountArray[i]->outFile, "w");
        if (flag) fprintf(stream, "account %d:\n", i); // For Duck Bank
        else fprintf(stream, "account: %d\n", i); // For Puddles Bank
        fflush(stream);
        fclose(stream);
    }
}


// This function is used to match the expected output on Canvas. The file on
// Canvas includes the initial balance for the savings accounts, but not the
// regular Duck Bank account. This function is only called by Puddles Bank.
void WriteInitialSavings(account **accountArray, int totalAccounts)
{
    FILE *stream;
    int i;

    for (i=0; i<totalAccounts; i++) {
        stream = fopen(accountArray[i]->outFile, "a");
        fprintf(stream, "Current Savings Balance  %.2f\n", 
            accountArray[i]->balance);
        fflush(stream);
        fclose(stream);
    }
}