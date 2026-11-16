#ifndef _EXEC_H_
#define _EXEC_H_

// Internal headers
#include "common.h"

// Function declarations
void reap(void);
void child(char *args[], int argsc);
void launch_program(char *args[], int argsc);
void child_redir_output(char *args[], int argsc, struct Redirection redir);
void child_redir_input(char *args[], int argsc, struct Redirection redir);
void launch_program_with_redirection(char *args[], int argsc);

#endif