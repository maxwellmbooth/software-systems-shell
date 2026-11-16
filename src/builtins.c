// Internal headers
#include "builtins.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void init_lwd(char lwd[]) {
    
    if(getcwd(lwd, MAX_PROMPT_LEN-6) == NULL){
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

int is_cd(const char *line) {
    while (*line == ' ') {
        line++;
    }

    return strncmp(line, "cd", 2) == 0 && (line[2] == '\0' || line[2] == ' ');
}

void run_cd(char *args[], int argsc, char lwd[]) {
    const char *destination;

    if (argsc == 1) {
        destination = getenv("HOME");
        if (destination == NULL || destination[0] == '\0') {
            fprintf(stderr, "cd: HOME is unset or empty\n");
            return;
        }
    } else if (argsc == 2) {
        destination = strcmp(args[1], "-") == 0 ? lwd : args[1];
    } else {
        fprintf(stderr, "Usage: cd [directory]\n");
        return;
    }

    char saved_wd[MAX_PROMPT_LEN - 6];
    if (getcwd(saved_wd, sizeof saved_wd) == NULL) {
        perror("getcwd");
        return;
    }

    if (chdir(destination) == -1) {
        perror("cd");
        return;
    }

    strcpy(lwd, saved_wd);
}