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
#include <errno.h>
#include <string.h>

// GNU Readline libraries 
#include <readline/readline.h>
#include <readline/history.h>

static void construct_shell_prompt(char shell_prompt[], int interactive);
static void read_command_line(char line[], int interactive);

// Main entry point
int main(int argc, char *argv[]) {
    // initialize builtin cwd
    builtins_init();

    static struct Batch batch;


    if (argc > 1) {
        if (parse_line(argv[1], &batch) == -1) {
            return 2;
        }

        // empty line
        if (batch.pipelinesc == 0) {
            return 0;
        }

        int status = exec_batch(&batch);
        return status;
    }

    int interactive = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    
    if (interactive) {
        using_history();
    }

    char line[MAX_LINE];

    while (1) {
        read_command_line(line, interactive);

        if (parse_line(line, &batch) == -1) {
            continue;
        }

        // empty line
        if (batch.pipelinesc == 0) {
            continue;
        }

       exec_batch(&batch);
    }
}

static void construct_shell_prompt(char shell_prompt[], int interactive) {
    char cwd[MAX_PATH_LEN];
    const char *prefix = "";
    const char *dir = "?";

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        const char *home_rel = strip_home(cwd);
        prefix = home_rel ? "~" : "";
        dir = home_rel ? home_rel : cwd;
    }

    if (interactive) {
    snprintf(shell_prompt, MAX_PROMPT_LEN,
             "\001" COLOR_BOLD COLOR_GREEN "\002" "[s3] "
             "\001" COLOR_CYAN "\002" "%s%s"
             "\001" COLOR_RESET "\002" " $ ",
             prefix, dir);
    return;
}

    snprintf(shell_prompt, MAX_PROMPT_LEN, COLOR_BOLD COLOR_GREEN "[s3] " COLOR_CYAN "%s%s" COLOR_RESET " $ ", prefix, dir);
    return;
}

///Prints a shell prompt and reads input from the user
static void read_command_line(char line[], int interactive) {
    char shell_prompt[MAX_PROMPT_LEN]; // creates/reserves space for the character array. It does not fill it yet.
    construct_shell_prompt(shell_prompt, interactive);

        if (interactive) {
        char *input = readline(shell_prompt);
        if (input == NULL) {
            putchar('\n');
            exit(0);
        }

        if (strlen(input) >= MAX_LINE) {
            fprintf(stderr, "s3: command too long\n");
            line[0] = '\0';
        } else {
            if (input[strspn(input, " \t")] != '\0') {
                add_history(input);
            }
            strcpy(line, input);
        }

        free(input);
        return;
    }
    
    printf("%s", shell_prompt); // %s - print a c string
    fflush(stdout); // flush stdout buffer before fgets blocks

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL) {
        if (feof(stdin)) {
            putchar('\n');
            exit(0);
        }

        fprintf(stderr, "s3: fgets: %s\n", strerror(errno));
        exit(1);
    }
    ///Remove newline (enter)
    line[strcspn(line, "\n")] = '\0'; // strip newline and change to terminator else truncate
}