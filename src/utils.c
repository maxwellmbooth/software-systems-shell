// Internal headers
#include "utils.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function definitions
const char *get_home_dir(void) {
    const char *home = getenv("HOME");
    if (home == NULL || home[0] == '\0') {
        return NULL;
    } else {
        return home;
    }
}

const char *strip_home(const char *path) {
    const char *home = get_home_dir();
    if (!home) return NULL;

    size_t hlen = strlen(home);
    if (hlen && (strncmp(path, home, hlen) == 0) && (path[hlen] == '/' || path[hlen] == '\0')) {
        return path + hlen;
    }

    return NULL;
}

int expand_tilde(const char *path, char *out, size_t outsize) {
    if (path[0] != '~' || (path[1] != '\0' && path[1] != '/')) return 0;

    const char *home = get_home_dir();
    if (home == NULL) return -1;

    int n = snprintf(out, outsize, "%s%s", home, path + 1);
    return (n < 0 || (size_t) n >= outsize) ? -1 : 1;
}