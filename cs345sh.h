/*
* Stelios Papamichail csd4020@csd.uoc.gr
*/

#define MAX_NUM_OF_COMMAND_WORDS 100 // usual num of maximum command arguments is 9 (but is system dependent)
#define MAX_NUM_OF_COMMANDS 10 // what could it be hmm
#define MAX_INPUT_SIZE 1000 // num of max chars to read
#define MAX_NUM_OF_REDIR_CMDS 2 // how many redirection-related commands the shell supports

/**
 * Counts how many times the given char is present 
 * in the given string.
 * @param input The string in which to look for
 * @param lookupChar The char whose occurences to count
 * @return The number of occurences of the given char
 **/
int countCharOccurences(char* input, char lookupChar);

/**
 * Parses the available command words in the given command and places
 * them in the given array.
 * @param input The initial string to split that contains the command.
 * @param parsedWords An array to every command word.
 * @return The number of words in the given command
 **/
int parseCommandWords(char *input, char** parsedWords);

/**
 * Parses the available commands in the given string and places
 * them in the given array.
 * @param input The initial string to split that contains the commands separated by ';'.
 * @param parsedWords An array  containing every parsed command.
 * @return The number of commands in the given input
 **/
int parseMultipleCommands(char *input, char **parsedCommands);

/**
 * Splits the given string at each pipe char ('|') and places
 * each command in the given array.
 * @param input The initial string to split
 * @param inptParsed An array containing the parsed commands
 * @return The number of commands in the given input.
 **/
int splitAtPipe(char *input, char** inptParsed);

/**
 * Handles the execution of custom commands (i.e. cd, exit).
 * @param cmdInfo An array containing the command to execute in the first position, and the arguments
 * to execute it with, in the rest of the array.
 * @param numOfArgs The num of argument in the given cmdInfo array. Used to perform cleanup upon exit.
 * @param input The original line read from the shell. Used to perform cleanup upone exit.
 * @return Returns 0 if the command couldn't be executed (i.e. not a custom command), or 1 otherwise.
 **/
int handleCustomCommands(char **command, int numOfArgs, char* input);

/**
 * Displays the shell prompt in the following format:
 * <user>@cs345sh/<dir>$ where <user> is the currently logged in
 * user and <dir> is the current working directory.
 **/
void displayPrompt();

/**
 * Executes the non-custom command contained in the first
 * position of the given array with the arguments
 * contained in the rest of the array.
 * @param An array containing the command to execute, along with
 * its arguments.
 * */
void execSystemCommand(char **args);

/**
 * Executes the commands contained in the given array
 * using pipes. A child process is created for each command
 * and the processes communicate using the created pipes.
 * This function supports multiple pipes.
 * @param input The initial input containing the commands and the pipe char(s).
 * @param commands An array containing the parsed commands to be executed.
 * */
void execPipedCommands(char *input, char **commands);

/**
 * Splits the given input string at the given delimeter, and places
 * the parsed commands in the commands array.
 * @param input The initial input string to split
 * @param commands An array that will contain the parsed commands
 * @param delim The delimeter at which to split the input string
 * */
void splitAtRedirectionDelim(char *input, char **commands, char *delim);

/**
 * Executes the commands contained in the given array
 * using redirection. A child process is created and depending
 * on the given delimeter, the function redirects the I/O accordingly.
 * This function does NOT support multiple redirection commands chained.
 * Important: delim == '>' will overwrite the contents of the file on the right.
 * delim == '>>' will append the output of the command to the end of the file on the right.
 * @param input The initial input string containing the commands and the redirection char(s).
 * @param commands An array containing the parsed commands to be executed.
 * @param delim The delimeter based on which the redirection of I/O will happen.
 * */
void execRedirectionCommands(char *input, char **commands, char *delim);

/**
 * Removes any trailing whitespace from the given string.
 * @param input The string to remove the whitespace from
 * @return A pointer at the beginning of the new string.
 */
char* removeLeadingWhitespace(char *input);

/**
 * Executes the given commands after parsing them according to 
 * their type (i.e. pipes, redirection, etc.).
 * @param input A line read from the shell containing commands to execute
 * */
void parseInput(char *input);