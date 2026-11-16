// Internal headers
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_command(char line[], char *args[]) {
    char *token = strtok(line, " \t"); // finds text up to the first space and also the space is replaced with '\0', ending the first string.
    int argsc = 0;
    while (token != NULL && argsc < MAX_ARGS - 1)
    {
        args[(argsc)++] = token;
        token = strtok(NULL, " \t"); // continues searching through the same input string.
    }
    
    args[argsc] = NULL; ///args must be null terminated
    return argsc;
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