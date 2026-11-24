// Internal headers
#include "exec.h"
#include "builtins.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Forward declarations
static void exec_pipeline(const struct Pipeline *pl);
static void exec_command(const struct Command *cmd);
static void exec_builtin(const struct Command *cmd, builtin_fn builtin);
static int redirect_pipes(const int pipe_fds[2], int last_read_fd);
static int redirect_io(const struct Command *cmd);

// Run through a batch to run every pipeline
void exec_batch(const struct Batch *bat) {
    for (int i = 0; i < bat->pipelinesc; i++) {
        exec_pipeline(&bat->pipelines[i]);
    }

    return;
}

// Run through a pipeline and execute every command
static void exec_pipeline(const struct Pipeline *pl) {
    // builtins run in the shell itself
    if (pl->commandsc == 1 && pl->commands[0].subshell == NULL) {
        builtin_fn builtin = builtin_find(pl->commands[0].args[0]);
        if (builtin) {
            exec_builtin(&pl->commands[0], builtin);
            return;
        }
    }
    
    // otherwise execute a command in child process
    pid_t pids[MAX_COMMANDS];
    int pran = 0;

    int pipe_fds[2] = {-1, -1};
    int last_read_fd = -1;

    fflush(NULL);

    for (int i = 0; i < pl->commandsc; i++) {
        pipe_fds[0] = pipe_fds[1] = -1;

        // create pipe if not last command
        if (i != pl->commandsc - 1) {
            if (pipe(pipe_fds) == -1) {
                fprintf(stderr, "s3: error creating pipe\n");
                return;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        } else if (pid == 0) {
            redirect_pipes(pipe_fds, last_read_fd);
            exec_command(&pl->commands[i]);
        }

        pids[pran++] = pid;

        // if last output existed, close it
        if (last_read_fd != -1) {
            close(last_read_fd);
        }

        // if a next pipe was made close write side and set last to current read side
        if (pipe_fds[1] != -1) {
            close(pipe_fds[1]);
        }
        last_read_fd = pipe_fds[0];
    }

    // reap all child processes
    for (int i = 0; i < pran; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    return;
}

// Execute a single command
static void exec_command(const struct Command *cmd) {
    if (redirect_io(cmd) == -1) {
        _exit(EXIT_FAILURE);
    }

    // execute subshell if used
    if (cmd->subshell != NULL) {
        execl("/proc/self/exe", "s3", cmd->subshell, (char *)NULL);
        fprintf(stderr, "s3: error executing subshell %s: %s\n", cmd->subshell, strerror(errno));
        _exit(127);
    }

    // will only run if no subshell
    builtin_fn builtin = builtin_find(cmd->args[0]);
    if (builtin) {
        int status = builtin(cmd->args, cmd->argsc);
        _exit(status);
    }
        
    // run execvp on child process (error will only print if execvp fails)
    execvp(cmd->args[0], cmd->args);
    fprintf(stderr, "s3: error executing %s: %s\n", cmd->args[0], strerror(errno));
    _exit(127);
}

// Execute a single builtin function
static void exec_builtin(const struct Command *cmd, builtin_fn builtin) {
    int in_fd = dup(STDIN_FILENO);
    int out_fd = dup(STDOUT_FILENO);

    // check for descriptors successfully copied
    if (in_fd == -1 || out_fd == -1) {
        fprintf(stderr, "s3: dup: %s\n", strerror(errno));
        if (in_fd  != -1) close(in_fd);
        if (out_fd != -1) close(out_fd);
        return;
    }

    if (redirect_io(cmd) == -1) {
        dup2(in_fd, STDIN_FILENO);
        dup2(out_fd, STDOUT_FILENO);
        close(in_fd);
        close(out_fd);
        return;
    }

    builtin(cmd->args, cmd->argsc);

    dup2(in_fd, STDIN_FILENO);
    dup2(out_fd, STDOUT_FILENO);
    close(in_fd);
    close(out_fd);

    return;
}

static int redirect_pipes(const int pipe_fds[2], int last_read_fd) {
    // if last pipe existed copy last read side to input
    if (last_read_fd != -1) {
        dup2(last_read_fd, STDIN_FILENO);
        close(last_read_fd);
    }

    // if next pipe is needed copy output to write side
    if (pipe_fds[1] != -1) {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
    }

    close(pipe_fds[0]);
    return 0;
}

static int redirect_io(const struct Command *cmd) {
    // open input file and redirect
    if (cmd->infile != NULL) {
        int fd = open(cmd->infile, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "s3: error opening %s: %s\n", cmd->infile, strerror(errno));
            return -1;
        }

        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    // open output file (checking if appending) and redirect
    if (cmd->outfile != NULL) {
        int fd = open(cmd->outfile, O_WRONLY | O_CREAT | (cmd->is_append ? O_APPEND : O_TRUNC), 0666);

        if (fd == -1) {
            fprintf(stderr, "s3: error opening %s: %s\n", cmd->outfile, strerror(errno));
            return -1;
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    return 0;
}