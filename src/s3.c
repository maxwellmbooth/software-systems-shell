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

int command_with_redirection(char line[]){
    return strchr(line, '<') != NULL || strchr(line, '>') != NULL;
}


struct Redirection find_redirection(char *args[], int argsc){
    for (int i = 0; i < argsc; i++) {
        if (strcmp(args[i], "<") == 0)
            return (struct Redirection){ .type = REDIR_INPUT, .index = i };
        else if (strcmp(args[i], ">") == 0)
            return (struct Redirection){ .type = REDIR_OUTPUT, .index = i };
        else if (strcmp(args[i], ">>") == 0)
            return (struct Redirection){ .type = REDIR_APPEND, .index = i };
        
}
    return (struct Redirection){ .type = REDIR_NONE, .index = -1 };
}

void child_redir_output(char *args[], int argsc, struct Redirection redir){
    
    if (redir.index <= 0 || redir.index + 1 >= argsc) {
        fprintf(stderr, "Expected a command and an output filename\n");
        _exit(1);
    }

    int flags = O_WRONLY | O_CREAT; // 0001 O_WRONGLY 0010 O_CREAT
    flags = flags | ((redir.type == REDIR_APPEND) ? O_APPEND : O_TRUNC);

    int fd = open(args[redir.index + 1], flags, 0666);
    if (fd == -1) {
        perror("open");
        _exit(1);
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        _exit(1);
    }

    if (fd != STDOUT_FILENO) {
        close(fd);
    }

    for (int i = redir.index; i + 2 < argsc; i++) {
        args[i] = args[i + 2];
    }

    argsc -= 2;
    args[argsc] = NULL;
    child(args, argsc); // Your existing helper calls execvp()
    }

void child_redir_input(char *args[], int argsc, struct Redirection redir){
    if (redir.index <= 0 || redir.index + 1 >= argsc) {
        fprintf(stderr, "Expected a command and an input filename\n");
        _exit(1);
    }
    
    int fd = open(args[redir.index + 1], O_RDONLY);
    if(fd == -1){
        perror("open");
        _exit(1);
    }

    if(dup2(fd, STDIN_FILENO) == -1){
        perror("dup2");
        _exit(1);
    }

    if(fd != STDIN_FILENO){
        close(fd);
    }

    for (int i = redir.index; i + 2 < argsc; i++) {
        args[i] = args[i + 2];
    }

    argsc -= 2;
    args[argsc] = NULL;
    child(args, argsc);
}

void launch_program_with_redirection(char *args[], int argsc){
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
        struct Redirection redir = find_redirection(args, argsc);
        if(redir.type == REDIR_OUTPUT || redir.type == REDIR_APPEND){
            child_redir_output(args, argsc, redir);
        }
        else if(redir.type == REDIR_INPUT){
            child_redir_input(args, argsc, redir);
        }
        else{
            perror("REDIR_NONE");
            exit(EXIT_FAILURE);
        }
    }
}