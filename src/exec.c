// Internal headers
#include "exec.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

// Forward declarations
static int exec_pipeline(const struct Pipeline *pl);
static pid_t exec_command(const struct Command *cmd);

void reap(void) {
    wait(NULL);
}

int exec_batch() {
    return 0;
}

static int exec_pipeline(const struct Pipeline *pl) {
    return 0;
}

static pid_t exec_command(const struct Command *cmd) {
    fflush(NULL); // flush output buffers before forking
    pid_t pid = fork();

    // run execvp on child process
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        if (execvp(cmd->args[0], cmd->args) == -1) {
            fprintf(stderr, "s3 [%s]: %s\n", cmd->args[0], strerror(errno));
            _exit(EXIT_FAILURE);
        }
    }

    return pid;
}