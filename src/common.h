#ifndef _COMMON_H_
#define _COMMON_H_

// Common constants
#define MAX_LINE 1024
#define MAX_PATH_LEN 256
#define MAX_PROMPT_LEN (MAX_PATH_LEN + 64)

#define MAX_TOKENS MAX_LINE
#define MAX_PIPELINES 16
#define MAX_COMMANDS 16
#define MAX_ARGS 128

// ANSI color codes
#define COLOR_BOLD "\033[1m"
#define COLOR_RESET "\033[0m"

#define COLOR_BLACK "\033[30m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

// Argument index enum
enum ArgIndex {
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

// Redirection enums
enum RedirectionType {
    REDIR_NONE,
    REDIR_INPUT,
    REDIR_OUTPUT,
    REDIR_APPEND
};

struct Redirection {
    enum RedirectionType type;
    int index;
};

#endif