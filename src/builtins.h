#ifndef _BUILTINS_H_
#define _BUILTINS_H_

// Internal headers
#include "common.h"


// Function declarations
void init_lwd(char lwd[]);
int is_cd(const char *line);
void run_cd(char *args[], int argsc, char lwd[]);

#endif