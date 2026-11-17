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

struct Parser {
    struct Token *tokens;
    int pos;
};

struct Command {
    char *args[MAX_ARGS];
    int argsc;
};

struct Pipeline {
    struct Command cmds[MAX_COMMANDS];
    int cmdsc;
};

struct Batch {
    struct Pipeline pipes[MAX_PIPELINES];
    int pipesc;
};


// Forward declarations
static int tokenise(char *line, struct Token tokens[], int max_tlen);
static int parse_batch(struct Parser *parser, struct Batch *batch);
static int parse_pipeline(struct Parser *p, struct Pipeline *pipe);
static int parse_command(struct Parser *p, struct Command *cmd);

// Functions
void reap(void) {
    wait(NULL);
}

int parse_line(char *line) {
    struct Token tokens[MAX_LINE];
    if (tokenise(line, tokens, MAX_LINE) == -1) {
        return -1;
    }

    if (tokens[0].type == TOK_END) {
        return 0;
    }

    struct Parser parser = {
        .tokens = tokens,
        .pos = 0
    };

    struct Batch batch;
    while (tokens[parser.pos].type != TOK_END) {
        parse_batch(&parser, &batch);
    }

    return 0;
}

// Produce a batch of pipelines from the tokens
static int parse_batch(struct Parser *p, struct Batch *bat) {
    bat->pipesc = 0;
    
    while (1) {
        if (bat->pipesc >= MAX_PIPELINES) {
            fprintf(stderr, "s3: too many pipelines\n");
            return -1;
        }

        if (parse_pipeline(p, &bat->pipes[bat->pipesc]) == -1) {
            return -1;
        }
        bat->pipesc++;

        if (p->tokens[p->pos].type != TOK_SEMI) {
            break;
        }
        p->pos++;

        if (p->tokens[p->pos].type == TOK_END) {
            break;
        }
    }

    return 0;
}

static int parse_pipeline(struct Parser *p, struct Pipeline *pipe) {
    pipe->cmdsc = 0;

    struct Command cmd;
    parse_command(p, &cmd);

    if (p->tokens[p->pos].type == TOK_SEMI) { // PLACEHOLDER
        pipe->cmds[pipe->cmdsc++] = cmd;
    }

    return 0;
}

static int parse_command(struct Parser *p, struct Command *cmd) {
    *cmd = (struct Command){0};

    while (p->tokens[p->pos].type == TOK_WORD) { // replace with switch for redirects
        if (cmd->argsc >= MAX_ARGS - 1) {
            fprintf(stderr, "s3: too many arguments\n");
            return -1;
        }

        cmd->args[cmd->argsc++] = p->tokens[p->pos++].word;
    }

    if (cmd->argsc == 0) {
        fprintf(stderr, "s3: expected a command\n");
        return -1;
    }

    cmd->args[cmd->argsc] = NULL;
    return 0;
}

struct Redirection find_redirection(char *args[], int argsc){
    for (int i = 0; i < argsc; i++) {
        if (strcmp(args[i], "<") == 0)
            return (struct Redirection) { .type = REDIR_INPUT, .index = i };
        else if (strcmp(args[i], ">") == 0)
            return (struct Redirection) { .type = REDIR_OUTPUT, .index = i };
        else if (strcmp(args[i], ">>") == 0)
            return (struct Redirection) { .type = REDIR_APPEND, .index = i };
        
    }
    return (struct Redirection) { .type = REDIR_NONE, .index = -1 };
}

// Creates a list of tokens from the input to allow for multiple operators
// Replace with an operator lookup table when expanding to cover more ops
static int tokenise(char *line, struct Token tokens[], int max_tlen) {
    char *p = line;
    int n = 0;

    while (*p != '\0') {
        if (n >= max_tlen - 1) {
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