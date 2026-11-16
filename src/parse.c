// Internal headers
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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