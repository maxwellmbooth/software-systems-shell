// Internal headers
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Defines
#define SPECIAL_CHARS " \t|;()<>"

// Enums
enum TokenType {
    TOK_WORD,
    TOK_REDIR_IN,
    TOK_REDIR_OUT,
    TOK_REDIR_APPEND,
    TOK_PIPE,
    TOK_SEMI,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_END
};

// Structs
struct Token {
    enum TokenType type;
    char *word;
};

void reap(void) {
    wait(NULL);
}

int parse_command(char line[], char *args[]) {
    char *token = strtok(line, " \t"); // finds text up to the first space and also the space is replaced with '\0', ending the first string.
    int argsc = 0;
    while (token != NULL && argsc < MAX_ARGS - 1)
    {
        args[(argsc)++] = token;
        token = strtok(NULL, " \t"); // continues searching through the same input string.
    }
    
    args[argsc] = NULL; ///args must be null terminated
    return argsc;
}

struct Redirection find_redirection(char *args[], int argsc){
    for (int i = 0; i < argsc; i++) {
        if (strcmp(args[i], "<") == 0)
            return (struct Redirection){ .type = REDIR_INPUT, .index = i };
        else if (strcmp(args[i], ">") == 0)
            return (struct Redirection){ .type = REDIR_OUTPUT, .index = i };
        else if (strcmp(args[i], ">>") == 0)
            return (struct Redirection){ .type = REDIR_APPEND, .index = i };
        
    }
    return (struct Redirection){ .type = REDIR_NONE, .index = -1 };
}

// Replace with an operator lookup table when expanding
static int tokenise(char *line, struct Token tokens[], int tlen) {
    char *p = line;
    int n = 0;

    while (*p != '\0') {
        if (n >= tlen - 1) {
            fprintf(stderr, "s3: too many tokens\n");
            return -1;
        }

        switch (*p) {
            case '<':
                tokens[n].type = TOK_REDIR_IN;
                *p++ = '\0';
                break;
            case '>':
                if (*(p + 1) == '>') {
                    tokens[n].type = TOK_REDIR_APPEND;
                    *p++ = '\0';
                    *p++ = '\0';
                    break;
                } else {
                    tokens[n].type = TOK_REDIR_OUT;
                    *p++ = '\0';
                    break;
                }
            case '|':
                tokens[n].type = TOK_PIPE;
                *p++ = '\0';
                break;
            case ';':
                tokens[n].type = TOK_SEMI;
                *p++ = '\0';
                break;
            case '(':
                tokens[n].type = TOK_LPAREN;
                *p++ = '\0';
                break;
            case ')':
                tokens[n].type = TOK_RPAREN;
                *p++ = '\0';
                break;
            case ' ': case '\t':
                *p++ = '\0';
                continue;
            default:
                tokens[n].type = TOK_WORD;
                tokens[n].word = p;
                p += strcspn(p, SPECIAL_CHARS);
                break;
        }
        n++;
    }

    tokens[n].type = TOK_END;
    return n;
}