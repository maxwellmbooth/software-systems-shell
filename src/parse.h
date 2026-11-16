#ifndef _PARSE_H_
#define _PARSE_H_

// Internal headers
#include "common.h"

// Function declarations
void parse_command(char line[], char *args[], int *argsc);
int command_with_redirection(char line[]);
struct Redirection find_redirection(char *args[], int argsc);

#endif