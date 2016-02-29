/**
 * @file quash.h
 *
 * Quash essential functions and structures.
 */

#ifndef QUASH_H
#define QUASH_H

#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * Specify the maximum number of characters accepted by the command string
 */
#define MAX_COMMAND_LENGTH (1024)
#define MAX_PATH_LENGTH (256)

/**
 * Holds information about a command.
 */
typedef struct command_t {
    char cmdstr[MAX_COMMAND_LENGTH]; ///< character buffer to store the
                                   ///< command string. You may want
                                   ///< to modify this to accept
                                   ///< arbitrarily long strings for
                                   ///< robustness.
    int execNumArgs;
    size_t cmdlen;     
    // Extend with more fields if needed
    bool execBg;//true if this is a background execution
    char* execArgs[MAX_PATH_LENGTH] ;
} command_t;

struct test_struct *ptr = NULL;

/**
 * Global variables.
 */
char PATH[MAX_PATH_LENGTH];
char HOME[MAX_PATH_LENGTH];

char WKDIR[MAX_PATH_LENGTH];

/**
*SIGCHLD handler
*/
void catchChild();

/**
 * Query if quash should accept more input or not.
 *
 * @return True if Quash should accept more input and false otherwise
 */
bool is_running();


/**
 * Causes the execution loop to end.
 */
void terminate();

/**
 * Prints the working directory.
 */
void pwd();

/**
 * changes the workign directory.
 */
void cd(command_t cmd);

/**
 * Tries to execute command
 *
 * @return 0 if success and error number if failure
 */
int exec_cmd(command_t cmd);

/**
 * Checks if the file at the path exists
 */
bool pathExists(char * path);

/**
 * Validates the execution path for a command
 */
void testPath(char * testPath, char * testReturn);

/**
 * Sets global variables
 */
void set(command_t cmd);

/**
 *  Read in a command and setup the #command_t struct. Also perform some minor
 *  modifications to the string to remove trailing newline characters.
 *
 *  @param cmd - a command_t structure. The #command_t.cmdstr and
 *               #command_t.cmdlen fields will be modified
 *  @param in - an open file ready for reading
 *  @return True if able to fill #command_t.cmdstr and false otherwise
 */
bool get_command(command_t* cmd, FILE* in);

#endif // QUASH_H
