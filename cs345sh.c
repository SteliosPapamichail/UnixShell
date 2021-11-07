/*
* Stelios Papamichail csd4020@csd.uoc.gr
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cs345sh.h"

/**
 * Counts how many times the given char is present 
 * in the given string.
 * @param input The string in which to look for
 * @param lookupChar The char whose occurences to count
 * @return The number of occurences of the given char
 **/
int countCharOccurences(char *input, char lookupChar)
{
    char *str = input;
    int count = 0;
    int i;
    for (i = 0; str[i]; i++)
    {
        if (str[i] == lookupChar)
            count++;
    }
    return count;
}

/**
 * Parses the available command words in the given command and places
 * them in the given array.
 * @param input The initial string to split that contains the command.
 * @param parsedWords An array to every command word.
 * @return The number of words in the given command
 **/
int parseCommandWords(char *input, char **parsedWords)
{
    int i;
    int cnt = 0;
    for (i = 0; i < MAX_NUM_OF_COMMAND_WORDS; i++)
    {
        char word[MAX_NUM_OF_COMMAND_WORDS];
        input = removeLeadingWhitespace(input);
        if (strlen(input) == 0)
            break;
        if (input[0] == '\"')
        {
            char *inptPtr = input + 1; // start after the beginning " char
            int charCnt = 0;
            while (inptPtr[0] != '\"')
            {
                inptPtr++;
                charCnt++;
            }
            if (charCnt >= MAX_NUM_OF_COMMAND_WORDS)
            {
                perror("Quoted argument was too long!\n");
                exit(EXIT_FAILURE);
            }
            strncpy(word, input + 1, charCnt); // input+1 : start after the beginning " and charCnt: end before the closing "
            word[charCnt] = '\0';              // add null terminator
            // check if there are chars left to parse or not
            if (strlen(++inptPtr) > 0)
            {
                input = inptPtr;
            }
            else
            {
                input = NULL;
            }
            parsedWords[i] = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
            cnt++;
            strcpy(parsedWords[i], word);
            if (input == NULL || strlen(input) == 0)
                return cnt;
            else
                continue;
        }
        strcpy(word, strsep(&input, " "));
        if (word == NULL)
            break;             // nothing to split
        if (strlen(word) == 0) // read an empty command, re-iterate
        {
            i--;
            continue;
        }
        parsedWords[i] = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!parsedWords[i])
        {
            perror("Failed to allocate memory for command\n");
            exit(EXIT_FAILURE);
        }
        cnt++;
        strcpy(parsedWords[i], word);
        if (input == NULL || strlen(input) == 0)
            break;
    }
    return cnt;
}

/**
 * Parses the available commands in the given string and places
 * them in the given array.
 * @param input The initial string to split that contains the commands separated by ';'.
 * @param parsedWords An array  containing every parsed command.
 * @return The number of commands in the given input
 **/
int parseMultipleCommands(char *input, char **parsedCommands)
{
    int numOfSemicolons = countCharOccurences(input, ';');
    int i;
    int cmdCnt = 0;
    for (i = 0; i < numOfSemicolons + 1; i++)
    {
        char *cmd = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!cmd)
        {
            perror("error allocating memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(cmd, strsep(&input, ";"));
        if (cmd == NULL)
            break;
        if (strlen(cmd) == 0)
        {
            free(cmd);
            i--;
            continue;
        }
        parsedCommands[i] = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!parsedCommands[i])
        {
            perror("Failed to allocate memory for every command in the chain\n");
            exit(EXIT_FAILURE);
        }
        char *trimmedCmd = removeLeadingWhitespace(cmd);
        strcpy(parsedCommands[i], trimmedCmd);
        cmdCnt++;
        free(cmd);
        if (input == NULL || strlen(input) == 0)
            break;
    }
    return cmdCnt;
}

/**
 * Removes any trailing whitespace from the given string.
 * @param input The string to remove the whitespace from
 * @return A pointer at the beginning of the new string.
 */
char *removeLeadingWhitespace(char *input)
{
    while (*input == ' ')
        input++;
    return input;
}

/**
 * Splits the given string at each pipe char ('|') and places
 * each command in the given array.
 * @param input The initial string to split
 * @param inptParsed An array containing the parsed commands
 * @return The number of commands in the given input.
 **/
int splitAtPipe(char *input, char **inptParsed)
{
    int numOfPipes = countCharOccurences(input, '|');
    int i;
    int cmdCnt = 0;
    // create a copy of the given input in order to preserve the original
    char *inpt = strdup(input);
    char *inptRef = inpt; // keep track of the original mem location since the inpt ptr will be changed by strsep
    for (i = 0; i < numOfPipes + 1; i++)
    {
        char *cmd = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!cmd)
        {
            free(inptRef);
            perror("error allocating memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(cmd, strsep(&inpt, "|"));
        if (cmd == NULL)
            break;
        if (strlen(cmd) == 0)
        {
            free(cmd);
            i--;
            continue;
        }
        inptParsed[i] = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!inptParsed[i])
        {
            free(inptRef);
            free(cmd);
            perror("Failed to allocate memory for every command in the chain\n");
            exit(EXIT_FAILURE);
        }
        char *trimmedCmd = removeLeadingWhitespace(cmd);
        strcpy(inptParsed[i], trimmedCmd);
        cmdCnt++;
        free(cmd);
        if (input == NULL || strlen(input) == 0)
            break;
    }
    free(inptRef);
    return cmdCnt;
}

/**
 * Handles the execution of custom commands (i.e. cd, exit).
 * @param cmdInfo An array containing the command to execute in the first position, and the arguments
 * to execute it with, in the rest of the array.
 * @param numOfArgs The num of argument in the given cmdInfo array. Used to perform cleanup upon exit.
 * @param input The original line read from the shell. Used to perform cleanup upone exit.
 * @return Returns 0 if the command couldn't be executed (i.e. not a custom command), or 1 otherwise.
 **/
int handleCustomCommands(char **cmdInfo, int numOfArgs, char* input)
{
    int numOfCustomCommands = 2;
    char *customCommands[numOfCustomCommands];
    customCommands[0] = "cd";
    customCommands[1] = "exit";
    int i;
    for (i = 0; i < numOfCustomCommands; i++)
    {
        // find the command to execute
        if (strcmp(cmdInfo[0], customCommands[i]) == 0)
            break;
    }

    switch (i)
    {
    case 0:
        if (chdir(cmdInfo[1]) == -1)
            return 0;
        else
            return 1;
    case 1:
        // free memory allocated for cmd/cmd args before we exit
        for (i = 0; i < numOfArgs; i++)
        {
            free(cmdInfo[i]);
        }
        free(input); // free mem allocated by getline
        exit(0);
        return 1;
    default:
        break;
    }
    return 0;
}

/**
 * Displays the shell prompt in the following format:
 * <user>@cs345sh/<dir>$ where <user> is the currently logged in
 * user and <dir> is the current working directory.
 **/
void displayPrompt()
{
    char *user = getlogin();
    char cwd[512]; // support up to 512 chars long dir paths
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("error retrieving current working directory.");
        exit(-1);
    }
    else if (user == NULL)
    {
        perror("error getting currently logged in user.");
        exit(-1);
    }
    else
    {
        printf("%s@cs345%s$ ", user, cwd);
    }
}

/**
 * Executes the non-custom command contained in the first
 * position of the given array with the arguments
 * contained in the rest of the array.
 * @param An array containing the command to execute, along with
 * its arguments.
 * */
void execSystemCommand(char **args)
{
    // create an identical child process
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("\nFailed to fork child..");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execvp(args[0], args) < 0)
        {
            perror("Could not execute given command..");
        }
        exit(EXIT_FAILURE);
    }
    else
    {
        // wait for the child process to finish
        wait(NULL);
        return;
    }
}

/**
 * Executes the commands contained in the given array
 * using pipes. A child process is created for each command
 * and the processes communicate using the created pipes.
 * @param input The initial input containing the commands and the pipe char(s).
 * @param commands An array containing the parsed commands to be executed.
 * */
void execPipedCommands(char *input, char **commands)
{
    int numOfPipes = countCharOccurences(input, '|');
    int fds[2 * numOfPipes]; // two file descriptors per pipe needed for interprocess communication
    int i;
    pid_t cpid;

    // initialize all pipes and store their respective fds in the appropriate place in the array
    for (i = 0; i < numOfPipes; i++)
    {
        if (pipe(fds + 2 * i) == -1)
        {
            perror("Failed to create file descriptors for pipe commands!\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < numOfPipes + 1; i++)
    {
        if (commands[i] == NULL)
            break;
        char *args[MAX_NUM_OF_COMMAND_WORDS] = {
            NULL,
        };
        int numOfArgs = parseCommandWords(commands[i], args);
        cpid = fork(); // start a child process
        if (cpid == -1)
        {
            perror("Failed to fork..\n");
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        { // child process is executing
            if (i != 0)
            { // if this is not the first command in the chain
                // duplicate the file descriptor to read from the previous command's output
                if (dup2(fds[(i - 1) * 2], STDIN_FILENO) < 0)
                {
                    perror("Failed to read input from previous command..\n");
                    exit(EXIT_FAILURE);
                }
            }

            // if this is not the last command in the chain
            if (i != numOfPipes && commands[i + 1] != NULL)
            {
                // duplicate write file descriptor in order to output to the next command
                if (dup2(fds[(i * 2 + 1)], STDOUT_FILENO) < 0)
                {
                    perror("Failed to write output for the next command..\n");
                    exit(EXIT_FAILURE);
                }
            }

            int j;
            for (j = 0; j < numOfPipes + 1; j++)
            { // close all copies of the file descriptors
                close(fds[j]);
            }
            // execute command
            if (execvp(args[0], args) < 0)
            {
                perror("Failed to execute piped command(s)");
                return;
            }
            perror("freeing args\n");
            for (i = 0; i < numOfArgs; i++)
                free(args[i]);
        }
    }
    // parent closes all original file descriptors
    for (i = 0; i < numOfPipes + 1; i++)
    {
        close(fds[i]);
    }

    // parent waits for all child processes to finish
    for (i = 0; i < numOfPipes + 1; i++)
        wait(0);
}

/**
 * Splits the given input string at the given delimeter, and places
 * the parsed commands in the commands array.
 * @param input The initial input string to split
 * @param commands An array that will contain the parsed commands
 * @param delim The delimeter at which to split the input string
 * */
void splitAtRedirectionDelim(char *input, char **commands, char *delim)
{
    // create a copy of the given input in order to preserve the original
    char *inpt = strdup(input);
    char *inptRef = inpt; // keep track of the original mem location since the inpt ptr will be changed by strtok
    char *cmd = strtok(inpt, delim);
    if (cmd == NULL || strlen(cmd) == 0)
    {
        free(inptRef);
        return;
    }
    // remove whitespace from the two commands and store them
    int i = 0;
    while (cmd != NULL)
    {
        // allocate memory for the command
        commands[i] = (char *)malloc(MAX_NUM_OF_COMMAND_WORDS * sizeof(char));
        if (!commands[i])
        {
            perror("Failed to allocate memory for commands\n");
            exit(EXIT_FAILURE);
        }
        char *trimmedCmd = removeLeadingWhitespace(cmd);
        strcpy(commands[i], trimmedCmd);
        cmd = strtok(NULL, delim);
        i++;
    }
    free(inptRef);
}

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
void execRedirectionCommands(char *input, char **commands, char *delim)
{
    if (strchr(input, '>')) // > || >>
    {
        pid_t cpid;
        int i;
        int fd;
        if (commands[0] == NULL || commands[1] == NULL)
            return;
        char *args[MAX_NUM_OF_COMMAND_WORDS] = {
            NULL,
        };
        int numOfArgs = parseCommandWords(commands[0], args);
        // commands[1] contains the file to write/append to
        if (strcmp(delim, ">>") == 0)
        {
            if ((fd = open(commands[1], O_CREAT | O_WRONLY | O_APPEND, 0644)) < 0)
            {
                perror("open for appending error\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // open for writting by overwritting prev data (trunc)
            if ((fd = open(commands[1], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0)
            {
                perror("open for writing error\n");
                exit(EXIT_FAILURE);
            }
        }

        cpid = fork(); // start a child process
        if (cpid == -1)
        {
            perror("Failed to fork..\n");
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        { // child process is executing

            if (dup2(fd, STDOUT_FILENO) < 0)
            {
                perror("dup2 failed\n");
                exit(EXIT_FAILURE);
            }
            close(fd);

            // execute command
            if (execvp(args[0], args) < 0)
            {
                perror("Failed to execute redirectional command(s)");
                exit(EXIT_FAILURE);
            }
            int i;
            for (i = 0; i < numOfArgs; i++)
                free(args[i]);
        }
        wait(NULL);
        close(fd);
    }
    else if (strcmp(delim, "<") == 0)
    {
        pid_t cpid;
        int i;
        int fd;
        if (commands[0] == NULL || commands[1] == NULL)
            return;
        char *args[MAX_NUM_OF_COMMAND_WORDS] = {
            NULL,
        };
        int numOfArgs = parseCommandWords(commands[0], args);
        // commands[1] contains the file to read from
        if ((fd = open(commands[1], O_RDONLY)) < 0)
        {
            perror("open for writing error\n");
            exit(EXIT_FAILURE);
        }
        cpid = fork(); // start a child process
        if (cpid == -1)
        {
            perror("Failed to fork..\n");
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        { // child process is executing

            if (dup2(fd, STDIN_FILENO) < 0)
            {
                perror("dup2 failed\n");
                exit(EXIT_FAILURE);
            }
            close(fd);

            // execute command
            if (execvp(args[0], args) < 0)
            {
                perror("Failed to execute redirectional command(s)");
                exit(EXIT_FAILURE);
            }
            int i;
            for (i = 0; i < numOfArgs; i++)
                free(args[i]);
        }
        wait(NULL);
        close(fd);
    }
}

/**
 * Executes the given commands after parsing them according to 
 * their type (i.e. pipes, redirection, etc.).
 * @param input A line read from the shell containing commands to execute
 * */
void parseInput(char *input)
{
    if (strchr(input, '|') != NULL)
    {
        // possibly piped command(s)
        char *commands[MAX_NUM_OF_COMMANDS] = {
            NULL,
        };
        int numOfCmds = splitAtPipe(input, commands);
        execPipedCommands(input, commands);
        int i;
        for (i = 0; i < numOfCmds; i++)
            if (commands[i] != NULL)
                free(commands[i]);
    }
    else if (strchr(input, '>') != NULL || strchr(input, '<') != NULL)
    { // no need to check for >> since we check for >
        // redirection commands
        char *commands[MAX_NUM_OF_REDIR_CMDS] = {
            NULL,
        };
        char *delim = (char *)malloc(3 * sizeof(char));

        if (strstr(input, ">>"))
            strcpy(delim, ">>");
        else if (strchr(input, '>'))
            strcpy(delim, ">");
        else
            strcpy(delim, "<");

        splitAtRedirectionDelim(input, commands, delim);
        execRedirectionCommands(input, commands, delim);

        int i;
        for (i = 0; i < MAX_NUM_OF_REDIR_CMDS; i++)
            if (commands[i] != NULL)
                free(commands[i]);
        free(delim);
    }
    else if (strchr(input, ';') != NULL)
    {
        // possibly multiple command(s)
        char *commands[MAX_NUM_OF_COMMANDS] = {
            NULL,
        };
        int numOfCmds = parseMultipleCommands(input, commands);
        int i;
        for (i = 0; i < numOfCmds; i++)
        {
            if (commands[i] == NULL)
                break;
            // single command
            char *args[MAX_NUM_OF_COMMAND_WORDS] = {
                NULL,
            };
            int numOfArgs = parseCommandWords(commands[i], args);
            if (handleCustomCommands(args,numOfArgs,input) == 0)
            {
                execSystemCommand(args);
            }
            int j;
            for (j = 0; j < numOfArgs; j++)
            {
                free(args[j]);
            }
            if (commands[i] != NULL)
                free(commands[i]);
        }
    }
    else
    {
        // single command
        char *args[MAX_NUM_OF_COMMAND_WORDS] = {
            NULL,
        };
        int numOfArgs = parseCommandWords(input, args);
        if (handleCustomCommands(args,numOfArgs,input) == 0)
        {
            execSystemCommand(args);
        }
        int i;
        for (i = 0; i < numOfArgs; i++)
        {
            free(args[i]);
        }
    }
}

int main()
{
    char *inputBuf = NULL; // getline will allocate the buffer
    size_t inputLen = 0;
    while (1)
    {
        displayPrompt();
        if (getline(&inputBuf, &inputLen, stdin) == -1)
        {
            perror("Error reading input.");
            exit(EXIT_FAILURE);
        }
        if (*inputBuf == '\n')
            continue;
        else
        {
            // remove the \n at the end of the read line ()
            inputBuf[strcspn(inputBuf, "\n")] = '\0';
            parseInput(inputBuf);
        }
    }
    return 0;
}