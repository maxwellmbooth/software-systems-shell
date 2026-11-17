#ifndef _PARSE_H_
#define _PARSE_H_

// Internal headers
#include "common.h"

// Structs
struct Command {
    char *args[MAX_ARGS];
    int argsc;
};

struct Pipeline {
    struct Command commands[MAX_COMMANDS];
    int commandsc;
};

struct Batch {
    struct Pipeline pipelines[MAX_PIPELINES];
    int pipelinesc;
};

// Function declarations
int parse_line(char *line, struct Batch *batch);
struct Redirection find_redirection(char *args[], int argsc);

#endif