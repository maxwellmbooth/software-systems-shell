// Internal headers
#include "exec.h"
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

void reap(void) {
    wait(NULL);
}

void launch_program(char *args[], int argsc) {
    ///Implement this function:

    if(argsc == 0){
        return;
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

///Launch related functions
void child(char *args[], int argsc) {
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.

    if(execvp(args[ARG_PROGNAME], args) == -1){
        
        perror("child failed");
        _exit(EXIT_FAILURE);

    }

}

void child_redir_output(char *args[], int argsc, struct Redirection redir) {
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

void child_redir_input(char *args[], int argsc, struct Redirection redir) {
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

void launch_program_with_redirection(char *args[], int argsc) {
    fflush(NULL); // flush output buffers before forking
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    } else if (pid == 0) {
        //child
        struct Redirection redir = find_redirection(args, argsc);
        if(redir.type == REDIR_OUTPUT || redir.type == REDIR_APPEND){
            child_redir_output(args, argsc, redir);
        }
        else if(redir.type == REDIR_INPUT){
            child_redir_input(args, argsc, redir);
        }
        else {
            fprintf(stderr, "s3: no redirection operator found\n");
            exit(EXIT_FAILURE);
        }
    }
}