// Internal headers
#include "common.h"
#include "utils.h"
#include "builtins.h"
#include "exec.h"
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void construct_shell_prompt(char shell_prompt[]);
void read_command_line(char line[]);

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    builtins_init();///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {

        read_command_line(line); ///Notice the additional parameter (required for prompt construction)
        argsc = parse_command(line, args);
        if (argsc <= 0) continue;

        builtin_fn builtin = builtin_find(args[0]);
        if (builtin) {
            builtin(args, argsc);
        } else if (find_redirection(args, argsc).type != REDIR_NONE) {
            ///Command with redirection
           launch_program_with_redirection(args, argsc);
           reap();
       } else {
           launch_program(args, argsc);
           reap();
       }
    }

    return 0;
}

void construct_shell_prompt(char shell_prompt[]) {
    char cwd[MAX_PATH_LEN];
    const char *prefix = "";
    const char *dir = "?";

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const char *home_rel = strip_home(cwd);
        prefix = home_rel ? "~" : "";
        dir = home_rel ? home_rel : cwd;
    }

    snprintf(shell_prompt, MAX_PROMPT_LEN, COLOR_BOLD COLOR_GREEN "[s3] " COLOR_CYAN "%s%s" COLOR_RESET " $ ", prefix, dir);
    return;
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[]) {
    char shell_prompt[MAX_PROMPT_LEN]; // creates/reserves space for the character array. It does not fill it yet.
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt); // %s - print a c string
    fflush(stdout); // flush stdout buffer to prevent pipeline capturing prompt

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL) {
        if (feof(stdin)) {
            putchar('\n');
            exit(0);
        }
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strcspn(line, "\n")] = '\0'; // strip newline and change to terminator else truncate
}