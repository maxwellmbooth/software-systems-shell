#ifndef _PARSE_H_
#define _PARSE_H_

// Internal headers
#include "common.h"

// Enums
enum Separator {
    SEP_AND,
    SEP_OR,
    SEP_SEMI,
    SEP_NONE
};

// Structs
struct Command {
    char *args[MAX_ARGS];
    int argsc;

    char *subshell;
    char *infile;
    char *outfile;
    int is_append;
};

struct Pipeline {
    struct Command commands[MAX_COMMANDS];
    int commandsc;
    enum Separator sep;
};

struct Batch {
    char line[MAX_LINE];
    char word_pool[WORD_POOL_SIZE];

    struct Pipeline pipelines[MAX_PIPELINES];
    int pipelinesc;
};

// Function declarations
int parse_line(char *line, struct Batch *batch);

#endif