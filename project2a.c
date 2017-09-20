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
#include <termios.h>

void getInput();
bool isCommandCharacter(char);
bool isArrowKey(char, int*);
void evaluateArrowKey(int);
void setupTerminalOS();
void restoreTerminalOS();
void changeDirectory(char **);
bool isChangeDirectoryCommand(char **);
bool processCommands(char **, char **);
void printDirectory();
bool isQuitCommand(char **);
bool getInputFromUser(char **, char **);
int *createPipe();
bool getCommands(char *, char **, char **);
bool bufferIsValid(char *);
void childProcess(bool, int *, char **);
void parentProcess(int, bool, int *, char **);
void executeCommand(char **);
void waitForChild(int);

struct termios origConfig;

int main()
{
	char *firstCommand[100];
    char *secondCommand[100];
	bool twoCommands;

	setupTerminalOS();

	while ( 1 )
	{
		printDirectory();

		getInput();

		//twoCommands = getInputFromUser(firstCommand, secondCommand);

		// if ( processCommands(firstCommand, secondCommand) )
		// 	break;
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

	restoreTerminalOS();

    return 0;
}

void setupTerminalOS()
{
	tcgetattr(0, &origConfig);

	struct termios newConfig = origConfig;

	newConfig.c_lflag &= ~(ICANON | ECHO);
	newConfig.c_cc[VMIN] = 2;
	newConfig.c_cc[VTIME] = 1;

	tcsetattr(0, TCSANOW, &newConfig);
}

void restoreTerminalOS()
{
	tcsetattr(0, TCSANOW, &origConfig);
}

void printDirectory()
{
    char *directory = (char *)malloc(100 * sizeof(char *));
	getcwd(directory, 100);
	printf("%s> ", directory);
	free(directory);
}

void getInput()
{
	char character;
	int *arrowKey;
	while ( character = getchar() )
	{
		if ( isCommandCharacter(character) )
			break;
		else if ( isArrowKey(character, arrowKey) )
			evaluateArrowKey(*arrowKey);

		putchar(character);
	}
}

bool isCommandCharacter(char character)
{
	bool isCommand;	
	switch ( character )
	{
		case '\n':
			isCommand = true;
			break;
		case 127:
			isCommand = true;
			break;
		case 8:
			isCommand = true;
			break;
		default:
			isCommand = false;
			break;
	}
	return isCommand;
}	

bool isArrowKey(char character, int *arrowKey)
{
	printf("is an arrow key");

	char tempChar = character;
	if ( tempChar == 27 )
		tempChar = getchar();
	if ( tempChar == 91 )
		tempChar = getchar();
	
	if ( tempChar == 68 )
	{
		*arrowKey = 68;
		return true;
	}
	else if ( tempChar == 67 )
	{
		*arrowKey = 67;
		return true;
	}
	else if ( tempChar == 65 )
	{
		*arrowKey = 65;
		return true;
	}
	else if ( tempChar == 66 )
	{
		*arrowKey = 66;
		return true;
	}

	return false;
}

void evaluateArrowKey(int arrowkey)
{
	printf("this is an arrowkey");
}

bool processCommands(char **firstCommand, char **secondCommand)
{
	if ( isQuitCommand(firstCommand) ) 
		return true;
	else if ( isChangeDirectoryCommand(firstCommand) )
		changeDirectory(firstCommand);

	return false;
}

bool isQuitCommand(char **command)
{
	return (strcmp(command[0], "quit") == 0);
}

bool isChangeDirectoryCommand(char **command)
{
	return (strcmp(command[0], "cd") == 0);
}

void changeDirectory(char **command)
{
	char *newDirectory = (char *)malloc(100 * sizeof(char *));
	newDirectory = command[1];
	if ( chdir(newDirectory) < 0 )
		perror(NULL);
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

int *createPipe()
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

// Configure the input (termios) 
// while not exited
// 	print prompt
// 	read input (break on up, down and \n) 
// 	if broken on up or down
// 		clear the current input
// 		execute appropriate command to generate corresponding message 
// 		print message
// 	else
// 		parse input into a command
// 		if cd command
// 			change the current working directory
// 		else if exit command 
// 			break
// 		else
// 			execute the command 
// restore the input configuration (termios)





