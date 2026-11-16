// Internal headers
#include "builtins.h"
#include "utils.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Global variables
static char lwd[MAX_PATH_LEN]; // The last working directory

// Forward declarations
static void builtin_cd(char *args[], int argsc);
static void builtin_exit(char *args[], int argsc);

// Function definitions
void builtins_init(void) {
    if(getcwd(lwd, MAX_PATH_LEN) == NULL){
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

builtin_fn builtin_find(char *name) {
    if (strcmp(name, "cd") == 0) {
        return builtin_cd;
    } else if (strcmp(name, "exit") == 0) {
        return builtin_exit;
    } else {
        return NULL;
    }
}

// maybe separate destination assigning and actual command?
static void builtin_cd(char *args[], int argsc) {
    const char *destination;
    char exp_path[MAX_PATH_LEN];

    if (argsc == 1) {
        destination = get_home_dir();
        if (destination == NULL) {
            fprintf(stderr, "cd: HOME is unset or empty\n");
            return;
        }
    } else if (argsc == 2) {
        if (strcmp(args[1], "-") == 0) {
            destination = lwd;
        } else {
            int expn = expand_tilde(args[1], exp_path, sizeof(exp_path));
            destination = (expn > 0) ? exp_path : args[1];
        }
    } else {
        fprintf(stderr, "Usage: cd [directory]\n");
        return;
    }

    char saved_wd[MAX_PATH_LEN];
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

static void builtin_exit(char *args[], int argsc) {
    if (argsc > 1) {
        exit(atoi(args[1]));
    } else {
        exit(0);
    }
}