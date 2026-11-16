#ifndef _PARSE_H_
#define _PARSE_H_

// Internal headers
#include "common.h"

// Function declarations
int parse_command(char line[], char *args[]);
struct Redirection find_redirection(char *args[], int argsc);

#endif