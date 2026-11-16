#ifndef _UTILS_H_
#define _UTILS_H_

// Internal headers
#include "common.h"

// Standard libraries
#include <stddef.h>

// Function declarations
const char *get_home_dir(void);
const char *strip_home(const char *path);
int expand_tilde(const char *path, char *out, size_t outsize);

#endif