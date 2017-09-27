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

void setupTerminalOS();
void restoreTerminalOS();
int printDirectory();
int getInput(char *, int);
bool isCharacterBackspace(char);
void backspaceCharacter(char *, int *, int *);
bool isCharacterArrow(char, int*);
int determineWhichArrowKey(char);
void arrowCharacter(int, char, int, int *);
void updateBuffer(char *, char, int *, int *);
bool parseBuffer(char *, char **, char **);
bool getCommands(char *, char **, char **);
bool processCommands(bool, char **, char **);
bool isQuitCommand(char **);
bool isChangeDirectoryCommand(char **);
void changeDirectory(char **);
void executeCommands(bool, char**, char**);
int *createPipe();
void childProcess(bool, int *, char **);
void parentProcess(int, bool, int *, char **);
void executeCommand(char **);
void waitForChild(int);

enum ArrowKeys
{
	UP,
	DOWN,
	RIGHT,
	LEFT,
	NUMOFARROWKEYS
};

struct termios origConfig;

int main()
{
	int directoryLength;
	char *firstCommand[100];
    char *secondCommand[100];
	bool twoCommands;

	setupTerminalOS();

	while ( 1 )
	{
		directoryLength = printDirectory();

		char *buffer = (char *)calloc(256, sizeof(char));
		if ( !getInput(buffer, directoryLength) )
			continue;

		twoCommands = parseBuffer(buffer, firstCommand, secondCommand);

		if ( processCommands(twoCommands, firstCommand, secondCommand) )
			break;
		
		free(buffer);
	}   

	restoreTerminalOS();

    return 0;
}

void setupTerminalOS()
{
	tcgetattr(0, &origConfig);

	struct termios newConfig = origConfig;

	newConfig.c_lflag &= ~(ICANON | ECHO);
	newConfig.c_cc[VMIN] = 1;
	newConfig.c_cc[VTIME] = 0;

	tcsetattr(0, TCSANOW, &newConfig);
}

void restoreTerminalOS()
{
	tcsetattr(0, TCSANOW, &origConfig);
}

int printDirectory()
{
    char *directory = (char *)malloc(100 * sizeof(char *));
	getcwd(directory, 100);
	printf("%s> ", directory);

	int directoryLength = strlen(directory);

	free(directory);

	return directoryLength;
}

int getInput(char *buffer, int directoryLength)
{
	int bufferIndex = 0;
	int cursorPosition = 0;
	int arrowKey;
	char character;
	
	while ( character = getchar() )
	{
		if ( character == '\n' )
			break;
		else if ( isCharacterBackspace(character) )
			backspaceCharacter(buffer, &bufferIndex, &cursorPosition);
		else if( isCharacterArrow(character, &arrowKey) )
		 	arrowCharacter(arrowKey, buffer[cursorPosition], bufferIndex, &cursorPosition);
		else
			updateBuffer(buffer, character, &bufferIndex, &cursorPosition);
	}

	putchar('\n');
	buffer[bufferIndex] = '\0';
	return bufferIndex;
}

bool isCharacterBackspace(char character)
{
	bool isBackspace = false;
	switch ( character )
	{
		case 127:
			isBackspace = true;
			break;
		case 8:
			isBackspace = true;
			break;
		default:
			break;
	}
	return isBackspace;
}

void backspaceCharacter(char *buffer, int *bufferIndex, int *cursorPosition)
{
	if ( (*cursorPosition) > 0 )
	{
		(*cursorPosition)--;
		(*bufferIndex)--;
		buffer[(*bufferIndex)] = ' ';
		printf("\b \b");
	}
}

bool isCharacterArrow(char character, int *arrowKey)
{
	char checkArrow = character;
	if ( checkArrow == 27 )
	{
		checkArrow = getchar();
		if ( checkArrow == 91 )
		{
			checkArrow = getchar();
			*arrowKey = determineWhichArrowKey(checkArrow);
			return true;
		}
	}
	return false;
}

int determineWhichArrowKey(char checkArrow)
{
	switch( checkArrow )
	{
		case 65:
			return UP;
		case 66:
			return DOWN;
		case 67:
			return RIGHT;
		case 68:
			return LEFT;
		default:
			break;
	}
	return 0;
}

void arrowCharacter(int arrowKey, char character, int bufferIndex, int *cursorPosition)
{
	switch( arrowKey )
	{
		case UP:
			break;
		case DOWN:
			break;
		case RIGHT:
			if ( bufferIndex > (*cursorPosition) )
			{
				putchar(character);
				(*cursorPosition)++;
			}
			break;
		case LEFT:
			if ( (*cursorPosition) > 0 )
			{
				putchar('\b');
				(*cursorPosition)--;
			}
			break;
		default:
			break;
	}
}

void updateBuffer(char *buffer, char character, int *bufferIndex, int *cursorPosition)
{
	putchar(character);
	buffer[(*bufferIndex)++] = character;
	(*cursorPosition)++;
}

bool parseBuffer(char *buffer, char **firstCommand, char **secondCommand)
{
    return getCommands(buffer, firstCommand, secondCommand);
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

bool processCommands(bool twoCommands, char **firstCommand, char **secondCommand)
{
	if ( isQuitCommand(firstCommand) ) 
		return true;
	else if ( isChangeDirectoryCommand(firstCommand) )
		changeDirectory(firstCommand);
	else
		executeCommands(twoCommands, firstCommand, secondCommand);

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

void executeCommands(bool twoCommands, char**firstCommand, char**secondCommand)
{
	int *fileDescriptor = createPipe();
	int pid = fork();
	if ( pid == 0 )
	    childProcess(twoCommands, fileDescriptor, firstCommand);
	else if ( pid > 0 )
	    parentProcess(pid, twoCommands, fileDescriptor, secondCommand);
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

// • Printing the prompt 									- done
// • Executing commands from the user 						- done
// • Implementing the quit command 							- done
// • Implementing the cd command 							- done
// • Implementing dir command 								- done
// • Implementing left and right keys 						- done
// • Properly implementing the delete and backspace keys 	- done
// • Implementing pause 									- done
// • Implementing history
// • Implementing the pipe operator							- done
// • Implementing custom command
