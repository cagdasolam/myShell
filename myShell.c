#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char history[10][100];  // history array
int historyCounter = 0; // record for how many entry in history

// parse the given command line arguments
void parse(char *line, char **argv)
{
    // remove the new line from end of line
    // if dont remove it execvp function dosent work properly
    char *p = strchr(line, '\n');
    if (p)
    {
        *p = 0;
    }

    while (*line != '\0') // if not the end of line
    {
        while (*line == ' ' || *line == '\n')
            *line++ = 0; // replace white spaces with 0
        *argv++ = line;  // save the argument position
        while (*line != '\0' && *line != ' ' && *line != '\n')
        {
            line++; // skip the argument
        }
    }
    *argv = '\0'; // mark the end of argument list
}

// history is a built-in function in shell-like program
// add the given command argument to history
// history is a simple array that implements queue struct
// keeps at most 10 entry at once
// type history in shell
void addHistory(char *line)
{
    // for better apperance remove the \n from input line
    char *p = strchr(line, '\n');
    if (p)
    {
        *p = 0;
    }

    if (historyCounter != 10) // if there is not 10 entry in history directly add end of history
    {
        strcpy(history[historyCounter], line); // copy input string to history[historyCounter]
        historyCounter++;                      // add 1 to counter
    }
    else
    {
        // if there is 10 entry in history should remove first entry in history
        // to do that I copy the entries to previous entry in order
        // so I copy the second to on first one than I delete it from history
        for (int i = 1; i < 10; i++)
        {
            for (int j = 0; j < (strlen(history[i])); j++)
            {
                strcpy(history[i - 1], history[i]);
            }
            strcpy(history[i], line);
        }
    }
}

// cd <directory> is a built-in function in shell-like program
// simply call chdir()
void change_dir(char *dir)
{
    char s[100];
    // change directory if it is a valid directory
    // if it is not it returns an error
    if (chdir(dir) == -1)
        perror("chdir");
    // set enviroment variable PWD to new directory
    setenv("PWD", getcwd(s, 100), 1);
}

// dir is a built-in function
// it prints current directory in shell-like program
void print_dir()
{
    char s[100];
    printf("%s\n", getcwd(s, 100));
}

// history is a built-in function in shell-like program
// printHistory prints entries that in history array
void printHistory()
{
    for (int i = 0; i < historyCounter; i++)
    {
        printf("[%d] %s\n", i + 1, history[i]);
    }
}

// check the command is contains ampersand ("&") operator
// if it is it returns 1, else returns 0
int checkForAmp(char **argv)
{
    char *amp = "&";
    for (int i = 0; i < 10; i++)
    {
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], amp) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

// execute the given command with using execvp()
// to do that it forks a child process
void executeCommand(char **argv)
{
    pid_t pid, wpid;
    int status;

    if ((pid = fork()) < 0) // fork a child process
    {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) // part of child process
    {
        if (execvp(*argv, argv) < 0) // execute the command
        {
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
    }
    else
    {
        if (checkForAmp(argv) == 0)            // check if it is a background process
        {                                      // if it is not wait for child process
            while ((wpid = wait(&status)) > 0) // parent process waits the child process
                ;
        }
    }
}

// check the command is contains pipe operator
// if it is it returns 1, else returns 0
int checkForPipe(char **argv)
{
    for (int i = 0; i < 10; i++)
    {

        char *del = "|";
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], del) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

// create two child process to exec left part and right part of pipe operation
void pipeOperation(char **argv)
{
    char *argvIn[10];
    char *argvOut[10];

    int index;
    char *del = "|";
    // in here first I tried to split argv array to 2 array for pipe
    for (int i = 0; i < 10; i++) // find the index of | operator
    {
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], del) == 0)
            {
                index = i;
            }
        }
    }

    for (int i = 0; i < 10; i++) // copy to argvIn array before | operator
    {
        if (argv[i] != NULL && i < index)
        {
            argvIn[i] = argv[i];
            argvIn[i + 1] = '\0';
        }
    }

    int j = 0;
    for (int i = index + 1; i < 10; i++) // copy to argvout array after | operator
    {
        if (argv[i] != NULL)
        {
            argvOut[j] = argv[i];
            argvOut[j + 1] = '\0';
            j++;
        }
    }

    // after split array time to create pipe
    // create 2 child process for pipeing
    // first child is assigned left part
    // second is right part

    int fd[2];        // create pipe with fd array
    pid_t pid1, pid2; // 2 child process

    if (pipe(fd) < 0) // check pipe
        perror("pipe error"), exit(1);

    else if ((pid1 = fork()) < 0) // first fork
        perror("fork error"), exit(1);
    else if (pid1 == 0) // part of the first child
    {
        dup2(fd[1], STDOUT_FILENO); // allocates a new file descriptor
        close(fd[0]);
        close(fd[1]);
        execvp(*argvIn, argvIn); // execute left part
        exit(1);                 // terminate child
    }
    else if ((pid2 = fork()) < 0) // second fork
        perror("fork error"), exit(1);
    else if (pid2 == 0) // part of the second part
    {
        dup2(fd[0], STDIN_FILENO); // allocates a new file descriptor
        close(fd[0]);
        close(fd[1]);
        execvp(*argvOut, argvOut); // execute right part
        exit(1);                   // terminate child
    }
    else
    {
        // close pipes
        close(fd[0]);
        close(fd[1]);
        int status1;
        int status2;
        // parent process waits child process
        int corpse1 = waitpid(pid1, &status1, 0);
        int corpse2 = waitpid(pid2, &status2, 0);
    }
}

int main()
{

    char line[100]; // the input line
    char *argv[10]; // the command line argument

    while (1)
    {
        printf("myShell> ");

        fgets(line, sizeof line, stdin); // get input from user via command line

        addHistory(line); // add input to history

        parse(line, argv); // parse given input

        // first check first element of argv array
        // to understand given input is built-in function
        // if it is not than try to execute given command
        if (strcmp("cd", argv[0]) == 0) // cd commands
        {
            // if a dir given second token is exist and length of it will be greater than 0
            // if a dir not given, change dir $HOME variable
            if (argv[1] != NULL)
            {
                change_dir(argv[1]);
            }
            else
            {
                change_dir(getenv("HOME"));
            }
        }
        else if (strcmp("dir", argv[0]) == 0) // if input "dir" call print_dir() func
        {
            print_dir();
        }
        else if (strcmp("history", argv[0]) == 0) // if "history" call printHistory()
        {
            printHistory();
        }
        else if (strcmp("bye", argv[0]) == 0) // if "bye" terminates program
        {
            exit(0);
        }
        else // other commands
        {
            if (checkForPipe(argv) == 1) // if it is a pipe op
            {
                pipeOperation(argv); // call pipeOperation()
            }
            else // if it is not pipe directly execute command
            {
                executeCommand(argv); // call executeCommand();
            }
        }
    }

    return 0;
}
