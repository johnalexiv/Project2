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
#include <ctype.h>

void setupTerminalOS();
void restoreTerminalOS();
void initializeHistory();
void printDirectory();
int getInput(char *);
bool isCharacterBackspace(char);
void backspaceCharacter(char *, int *, int *);
bool isCharacterArrow(char, int *);
int determineWhichArrowKey(char);
void arrowCharacter(int, char *, int *, int *);
void upArrow(char *, int *, int *);
void downArrow(char *, int *, int *);
void rightArrow(char *, int *, int *);
void leftArrow(int *);
void updateBuffer(char *, char, int *, int *);
void addBufferToHistory(char *);
bool parseBuffer(char *, char **, char **);
bool getCommands(char *, char **, char **);
bool processCommands(bool, char **, char **);
bool isQuitCommand(char **);
void quitCommand(bool *);
bool isChangeDirectoryCommand(char **);
void changeDirectory(char **);
bool isMergeCommand(char **);
void mergeCommand(char **);
void mergeFiles(char **, char *);
void childMergeProcess(char **, char *);
bool isPauseCommand(char **);
void pauseCommand();
void executeCommands(bool, char **, char **);
int *createPipe();
void grandChildProcess(bool, int *, char **);
void childProcess(int, bool, int *, char **);
void executeCommand(char **);
void waitForChild(int);

struct History
{
    int index;
    int length;
    char *command[6];
}history = { 0, 0 };

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
    char *firstCommand[100];
    char *secondCommand[100];
    bool twoCommands;

    setupTerminalOS();

    initializeHistory();

    while ( 1 )
    {
        printDirectory();

        char *buffer = (char *)calloc(256, sizeof(char));
        if ( !getInput(buffer) )
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

void initializeHistory()
{
    int size = 0;
    while( size != 6 )
        history.command[size++] = (char *)calloc(1024, sizeof(char *));
}

void printDirectory()
{
    char *directory = (char *)malloc(100 * sizeof(char *));
    getcwd(directory, 100);
    printf("%s> ", directory);

    free(directory);
}

int getInput(char *buffer)
{
    int bufferIndex = 0;
    int cursorPosition = 0;
    int arrowKey;
    char character;
    history.index = 0;

    while ( character = getchar() )
    {
        if ( character == '\n' )
            break;
        else if ( isCharacterBackspace(character) )
            backspaceCharacter(buffer, &bufferIndex, &cursorPosition);
        else if( isCharacterArrow(character, &arrowKey) )
            arrowCharacter(arrowKey, buffer, &bufferIndex, &cursorPosition);
        else
            updateBuffer(buffer, character, &bufferIndex, &cursorPosition);
    }

    putchar('\n');
    buffer[bufferIndex] = '\0';

    addBufferToHistory(buffer);

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
    return -1;
}

void arrowCharacter(int arrowKey, char *buffer, int *bufferIndex, int *cursorPosition)
{
    switch( arrowKey )
    {
    case UP:
        upArrow(buffer, bufferIndex, cursorPosition);
        break;
    case DOWN:
        downArrow(buffer, bufferIndex, cursorPosition);
        break;
    case RIGHT:
        rightArrow(buffer, bufferIndex, cursorPosition);
        break;
    case LEFT:
        leftArrow(cursorPosition);
        break;
    default:
        break;
    }
}

void upArrow(char *buffer, int *bufferIndex, int *cursorPosition)
{
    if ( history.index < history.length )
    {
        int i;
        for( i = 0; i < *bufferIndex; i++ )
            printf("\b \b");

        strcpy(buffer, history.command[++history.index]);
        *bufferIndex = strlen(buffer);
        *cursorPosition = *bufferIndex;
        printf("%s", buffer);
    }
}

void downArrow(char *buffer, int *bufferIndex, int *cursorPosition)
{
    if ( history.index > 0 )
    {
        int i;
        for( i = 0; i < *bufferIndex; i++ )
            printf("\b \b");

        strcpy(buffer, history.command[--history.index]);
        *bufferIndex = strlen(buffer);
        *cursorPosition = *bufferIndex;
        printf("%s", buffer);
    }
}

void rightArrow(char *buffer, int *bufferIndex, int *cursorPosition)
{
    if ( *bufferIndex > (*cursorPosition) )
    {
        putchar(buffer[(*cursorPosition)]);
        (*cursorPosition)++;
    }
}

void leftArrow(int *cursorPosition)
{
    if ( (*cursorPosition) > 0 )
    {
        putchar('\b');
        (*cursorPosition)--;
    }
}

void updateBuffer(char *buffer, char character, int *bufferIndex, int *cursorPosition)
{
    putchar(character);
    buffer[(*bufferIndex)++] = character;
    (*cursorPosition)++;
    strcpy(history.command[0], buffer);
}

void addBufferToHistory(char *buffer)
{
    int i;
    for ( i = history.length; i > 0; i-- )
    {
        strcpy(history.command[i], history.command[i - 1]);
    }
    if ( history.length < 5 )
        history.length++;
    strcpy(history.command[1], buffer); 
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
    bool quit = false;
    if ( isQuitCommand(firstCommand) ) 
        quitCommand(&quit);
    else if ( isChangeDirectoryCommand(firstCommand) )
        changeDirectory(firstCommand);
    else if ( isMergeCommand(firstCommand) )
        mergeCommand(firstCommand);
    else if ( isPauseCommand(firstCommand) )
        pauseCommand();
    else
        executeCommands(twoCommands, firstCommand, secondCommand);

    return quit;
}

bool isQuitCommand(char **command)
{
    return (strcmp(command[0], "quit") == 0);
}

void quitCommand(bool *quit)
{
    printf("Are you sure you want to quit? (y/n): ");
    if( tolower(getchar()) == 'y' )
        *quit = true;
    putchar('\n');
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

bool isMergeCommand(char **command)
{
    if ( strcmp(command[0], "merge") == 0 )
        if ( strcmp(command[3], ">") == 0 )
            return true;
    return false;
}

void mergeCommand(char **command)
{
    char *firstCommand[100];
    firstCommand[0] = "cat";
    firstCommand[1] = command[1];
    firstCommand[2] = command[2];
    firstCommand[3] = NULL;
    char *mergeFile = command[4];
    mergeFiles(firstCommand, mergeFile);
}

void mergeFiles(char **firstCommand, char *mergeFile)
{
    int child = fork();
    if ( child == 0 )
        childMergeProcess(firstCommand, mergeFile);
    else if ( child > 0 )
        waitForChild(child);
    else 
        perror(NULL);
}

void childMergeProcess(char **command, char *mergeFile)
{
    FILE *outFile = fopen(mergeFile, "w");
    int fileDescriptor = fileno(outFile);
    close(1);
    dup(fileDescriptor);
    executeCommand(command);
    exit(0);
}

bool isPauseCommand(char **command)
{
    return (strcmp(command[0], "pause") == 0);
}

void pauseCommand()
{
    while(getchar() != '\n');
}

void executeCommands(bool twoCommands, char**firstCommand, char**secondCommand)
{
    int child;
    int grandChild;
    child = fork();
    if ( child == 0 )
    {
        int *fileDescriptor = createPipe();
        grandChild = fork();
        if ( grandChild == 0 )
            grandChildProcess(twoCommands, fileDescriptor, firstCommand);
        else if ( grandChild > 0 )
            childProcess(grandChild, twoCommands, fileDescriptor, secondCommand);
    }
    else if ( child > 0 )
        waitForChild(child);
    else
        perror(NULL);
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

void grandChildProcess(bool twoCommands, int *fileDescriptor, char **firstCommand)
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

void childProcess(int pid, bool twoCommands, int *fileDescriptor, char **secondCommand)
{
    waitForChild(pid);
    if( twoCommands )
    {
        close(0);
        dup(fileDescriptor[0]);
        close(fileDescriptor[1]);
        executeCommand(secondCommand);
    }
    exit(0);
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

