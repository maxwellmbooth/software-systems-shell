// Internal headers
#include "parse.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Defines
#define SPECIAL_CHARS " \t&|;()<>"
#define NUM_OPERATORS (sizeof(operators) / sizeof(operators[0]))

// Enums
enum TokenType {
    TOK_WORD,
    TOK_REDIR_APPEND,
    TOK_AND,
    TOK_OR,
    TOK_REDIR_OUT,
    TOK_REDIR_IN,
    TOK_PIPE,
    TOK_BG,
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

struct TokenList {
    struct Token tokens[MAX_TOKENS];
    int toksc;
};

struct Parser {
    struct Token *tokens;
    int pos;
};

// Variables and Tables
static const struct {
    const char *text;
    enum TokenType type;
} operators[] = {
    {">>",TOK_REDIR_APPEND},
    {"&&", TOK_AND},
    {"||", TOK_OR},
    {">", TOK_REDIR_OUT},
    {"<", TOK_REDIR_IN},
    {"|", TOK_PIPE},
    {"&", TOK_BG},
    {";", TOK_SEMI},
    {"(", TOK_LPAREN},
    {")", TOK_RPAREN},
};

// Forward declarations
static int parse_batch(struct Parser *parser, struct Batch *batch);
static int parse_pipeline(struct Parser *p, struct Pipeline *pipe);
static int parse_command(struct Parser *p, struct Command *cmd);
static int tokenise(char *line, struct TokenList *toks);
static char *read_word(char *p);
static size_t operator_at(const char *p, enum TokenType *type);

// Parse a line of shell input
int parse_line(char *line, struct Batch *batch) {
    batch->pipelinesc = 0;

    // tokenise line
    struct TokenList token_list;
    if (tokenise(line, &token_list) == -1) {
        return -1;
    }

    // check if empty line
    if (token_list.toksc == 0) {
        return 0;
    }

    // parse line
    struct Parser parser = {
        .tokens = token_list.tokens,
        .pos = 0
    };

    if (parse_batch(&parser, batch) == -1) {
        return -1;
    }

    // check for unparsed token
    if (token_list.tokens[parser.pos].type != TOK_END) {
        fprintf(stderr, "s3: unexpected token\n");
        return -1;
    }

    return 0;
}

// Produce a batch of pipelines from the tokens
static int parse_batch(struct Parser *p, struct Batch *bat) {
    bat->pipelinesc = 0;
    
    while (1) {
        if (bat->pipelinesc >= MAX_PIPELINES) {
            fprintf(stderr, "s3: too many pipelines\n");
            return -1;
        }

        // add pipeline to batch
        if (parse_pipeline(p, &bat->pipelines[bat->pipelinesc]) == -1) {
            return -1;
        }
        bat->pipelinesc++;

        // keep going until no more pipeline separators are present
        if (p->tokens[p->pos].type != TOK_SEMI) {
            break;
        }
        p->pos++;

        // check if next token is end as trailing ; is permitted
        if (p->tokens[p->pos].type == TOK_END) {
            break;
        }
    }

    return 0;
}

// Produce pipeline containing commands
static int parse_pipeline(struct Parser *p, struct Pipeline *pl) {
    pl->commandsc = 0;

    while (1) {
        if (pl->commandsc >= MAX_COMMANDS) {
            fprintf(stderr, "s3: too many commands\n");
            return -1;
        }

        // add command to pipeline
        if (parse_command(p, &pl->commands[pl->commandsc]) == -1) {
            return -1;
        }
        pl->commandsc++;

        // keep going until no more command separators are present
        if (p->tokens[p->pos].type != TOK_PIPE) {
            break;
        }
        p->pos++;
    }

    return 0;
}

// Produce command containing argument info
static int parse_command(struct Parser *p, struct Command *cmd) {
    *cmd = (struct Command){0};

    while (1) {
        const struct Token *t = &p->tokens[p->pos];

        switch (t->type) {
            case TOK_WORD: {
                if (cmd->argsc >= MAX_ARGS - 1) {
                    fprintf(stderr, "s3: too many arguments\n");
                    return -1;
                }
            
                cmd->args[cmd->argsc++] = t->word;
                p->pos++;
                continue;
            }
            case TOK_REDIR_IN:
            case TOK_REDIR_OUT:
            case TOK_REDIR_APPEND: {
                const struct Token *file = &p->tokens[p->pos + 1];
                if (file->type != TOK_WORD) {
                    fprintf(stderr, "s3: expected a filename\n");
                    return -1;
                }
                if (t->type == TOK_REDIR_IN) {
                    cmd->infile = file->word;
                } else {
                    cmd->outfile = file->word;
                    cmd->is_append = (t->type == TOK_REDIR_APPEND);
                }
                p->pos += 2;
                continue;
            }
            default:
                break;
        }
        break;
    }

    if (cmd->argsc == 0) {
        fprintf(stderr, "s3: expected a command\n");
        return -1;
    }

    cmd->args[cmd->argsc] = NULL;
    return 0;
}

// Creates a list of tokens from the input to allow for multiple operators
static int tokenise(char *line, struct TokenList *toks) {
    toks->toksc = 0;
    char *p = line;

    while (*p != '\0') {
        if (toks->toksc >= MAX_TOKENS - 1) {
            fprintf(stderr, "s3: too many tokens\n");
            return -1;
        }

        // null terminate words
        if (*p == ' ' || *p == '\t') {
            *p++ = '\0';
            continue;
        }

        struct Token *t = &toks->tokens[toks->toksc];
        *t = (struct Token){0};

        enum TokenType type;
        size_t tlen = operator_at(p, &type);

        // set operators to null after read for word termination and add token type/word
        if (tlen > 0) {
            t->type = type;
            memset(p, '\0', tlen);
            p += tlen;
        } else {
            t->type = TOK_WORD;
            t->word = p;
            p = read_word(p);
            if (p == NULL) {
                return -1;
            }
        }
        
        toks->toksc++;
    }

    toks->tokens[toks->toksc].type = TOK_END;
    return 0;
}

// Read a word starting from p, removing quotes and keeping symbols inside quotes
static char *read_word(char *p) {
    char *next = p; // pointer to next character to write to (as letters must be shifted when quotes take up space)
    char qchar = '\0';

    while (*p != '\0') {
        if (qchar != '\0') {
            if (*p == qchar) {
                qchar = '\0';
                p++;
                continue;
            }
        } else if (*p == '\'' || *p == '"') {
            qchar = *p++;
            continue;
        } else if (strchr(SPECIAL_CHARS, *p) != NULL) {
            break;
        }

        *next++ = *p++; // write the next character
    }

    if (qchar != '\0') {
        fprintf(stderr, "s3: mismatched quote\n");
        return NULL;
    }

    // null terminate string earlier if quotes removed
    if (next != p) {
        *next = '\0';
    }

    return p;
}

// Get the operator at the current position, return its length and set the type
static size_t operator_at(const char *p, enum TokenType *type) {
    for (size_t i = 0; i < NUM_OPERATORS; i++) {
        size_t n = strlen(operators[i].text);
        if (strncmp(p, operators[i].text, n) == 0) {
            *type = operators[i].type;
            return n;
        }
    }
    return 0;
}