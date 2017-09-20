// Project 2
// John Alexander
// CS 370
// Sept. 18, 2017

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h> 
#include <stdbool.h>

void changeDirectory(char **);
bool isChangeDirectoryCommand(char **);
bool processCommands(char **, char **);
void printDirectory();
bool isQuitCommand(char **);
bool getInputFromUser(char **, char **);
int * createPipe();
bool getCommands(char *, char **, char **);
bool bufferIsValid(char *);
void childProcess(bool, int *, char **);
void parentProcess(int, bool, int *, char **);
void executeCommand(char **);
void waitForChild(int);

int main()
{
	char *firstCommand[100];
    char *secondCommand[100];
	bool twoCommands;

	while(1)
	{
		printDirectory();

		twoCommands = getInputFromUser(firstCommand, secondCommand);

		if ( processCommands(firstCommand, secondCommand) )
			break;

	}   

    // char *firstCommand[100];
    // char *secondCommand[100];

    // bool twoCommands = getInputFromUser(firstCommand, secondCommand);
    
    // int *fileDescriptor = createPipe();
    // int pid = fork();
    // if ( pid == 0 )
    //     childProcess(twoCommands, fileDescriptor, firstCommand);
    // else if ( pid > 0 )
    //     parentProcess(pid, twoCommands, fileDescriptor, secondCommand);
    // else if ( pid < 0 )
    //     perror(NULL);

    return 0;
}

bool processCommands(char **firstCommand, char **secondCommand)
{
	if ( isQuitCommand(firstCommand) ) 
		return true;

	if ( isChangeDirectoryCommand(firstCommand) )
		changeDirectory(firstCommand);
}

void changeDirectory(char **command)
{
	char *newDirectory = (char *)malloc(100 * sizeof(char *));
	newDirectory = command[1];
	
}

bool isChangeDirectoryCommand(char **command)
{
	return (strcmp(command[0], "cd") == 0);
}

void printDirectory()
{
    char *directory = (char *)malloc(100 * sizeof(char *));
	getcwd(directory, 100);
	printf("%s> ", directory);
	free(directory);
}

bool isQuitCommand(char **command)
{
	return (strcmp(command[0], "quit") == 0);
}

bool getInputFromUser(char **firstCommand, char **secondCommand)
{
    bool twoCommands;
    char *buffer = (char *)calloc(256, sizeof(char));
    if ( bufferIsValid(buffer) )
    {
        printf("No command was entered. \n");
        exit(1);
    }
    else 
    {
        twoCommands = getCommands(buffer, firstCommand, secondCommand);
    }

    return twoCommands;
}

bool bufferIsValid(char *buffer)
{
    fgets(buffer, 256, stdin);
    buffer[strlen(buffer) - 1] = '\0';
    if ( buffer[0] == '\0' )
        return true;
    else 
        return false;
}

bool getCommands(char *buffer, char **firstCommand, char **secondCommand)
{
    bool twoCommands = false;
    int firstSize = 0;
    int secondSize = 0;

    char *token = strtok(buffer, " ");
    while( token != NULL )
    {
        if ( *token == '|' )
            twoCommands = true;
        else if ( !twoCommands )
            firstCommand[firstSize++] = token;      
        else
            secondCommand[secondSize++] = token;

        token = strtok(NULL, " ");
    }
    firstCommand[firstSize] = NULL;
    secondCommand[secondSize] = NULL;

    return twoCommands;
}

int * createPipe()
{
    int *fileDescriptor = (int *)malloc(2 * sizeof(int *));
    if ( pipe(fileDescriptor) < 0)
    {
        perror(NULL);
        exit(1);
    }
    return fileDescriptor;
}

void childProcess(bool twoCommands, int *fileDescriptor, char **firstCommand)
{
    if ( twoCommands )
    {
        close(1);
        dup(fileDescriptor[1]);
        close(fileDescriptor[0]);
    }
    executeCommand(firstCommand);
    exit(0);
}

void parentProcess(int pid, bool twoCommands, int *fileDescriptor, char **secondCommand)
{
    waitForChild(pid);
    if( twoCommands )
    {
        close(0);
        dup(fileDescriptor[0]);
        close(fileDescriptor[1]);
        executeCommand(secondCommand);
    }
}

void executeCommand(char **command)
{
    if ( execvp(command[0], command) < 0 )
    {
        perror(NULL);
        exit(1);
    }
}

void waitForChild(int pid)
{
    int status;
    if ( waitpid(pid, &status, WUNTRACED) < 0)
    {
        perror(NULL);
        exit(1);
    }
}





