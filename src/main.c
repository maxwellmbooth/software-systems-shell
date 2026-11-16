// Internal headers
#include "common.h"
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

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {

        read_command_line(line); ///Notice the additional parameter (required for prompt construction)

        if(is_cd(line)){///Implement this function
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd); ///Implement this function
        }
        else if(command_with_redirection(line)){
            ///Command with redirection
           parse_command(line, args, &argsc);
           launch_program_with_redirection(args, argsc);
           reap();
       }
       else ///Basic command
       {
           parse_command(line, args, &argsc);
           launch_program(args, argsc);
           reap();
       }
    }

    return 0;
}

void construct_shell_prompt(char shell_prompt[]){
    char cwd[MAX_PATH_LEN];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        snprintf(shell_prompt, MAX_PROMPT_LEN, COLOR_BOLD COLOR_GREEN "[s3] " COLOR_RESET "$ ");
        return;
    }

    snprintf(shell_prompt, MAX_PROMPT_LEN, COLOR_BOLD COLOR_GREEN "[s3] " COLOR_CYAN "%s" COLOR_RESET " $ ", cwd);
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[]) {
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