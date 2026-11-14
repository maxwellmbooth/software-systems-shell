#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[]) 
{
    char shell_prompt[MAX_PROMPT_LEN]; // creates/reserves space for the character array. It does not fill it yet.
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt); // %s - print a c string

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL) // fgets returns a pointer to line if reading succeeded;
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0'; // code assumes the final character is always a newline may need to change for pipeline
}

void parse_command(char line[], char *args[], int *argsc) // used to split input into args[] and count number of values
// in practice we pass int *argsc by refrence
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " "); // finds text up to the first space and also the space is replaced with '\0', ending the first string.
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " "); // continues searching through the same input string.
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.

    if(execvp(args[ARG_PROGNAME], args) == -1){
        
        perror("child failed");
        _exit(EXIT_FAILURE);

    }

}

void launch_program(char *args[], int argsc)
{
    ///Implement this function:

    if(argsc == 0){
        return;
    }
    if(strcmp(args[ARG_PROGNAME], "exit") == 0 ){
        exit(0);
    }

    pid_t pid = fork();
    if(pid<0){
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){
        //child
        child(args, argsc);
    }
    ///fork() a child process.
    ///In the child part of the code,
    ///call child(args, argv)
    ///For reference, see the code in lecture 2.
    ///Handle the 'exit' command;
    ///so that the shell, not the child process,
    ///exits.
}