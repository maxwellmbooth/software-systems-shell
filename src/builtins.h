#ifndef _BUILTINS_H_
#define _BUILTINS_H_

// Internal headers
#include "common.h"

// Builtin function type
typedef int (*builtin_fn)(char *const args[], int argsc);

// Function declarations
void builtins_init(void);
builtin_fn builtin_find(char *name);

#endif