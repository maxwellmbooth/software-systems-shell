# s3 Shell
## Feature Overview
- Support for **multiple redirects** for each command
- Support for **batched** commands
- `cd` and `exit` **built-in** support
- Support for **pipes** between commands
- Support for **subshells**
- Fully configurable maximums in `common.h`
## Tokeniser and Parser
- Each line tokenised appropriately allowing for easy operator handling
- Produces a command batch containing an array of pipelines
    - Each pipeline contains an array of commands
## Executor
- Execution takes batch produced by parser
- Redirection of both IO and pipes between commands
## Built-ins
- `cd` support
    - Also supports using `~` for home
- `exit` support
## Source File Descriptions
- `main.c`: Entry point and prompt functions
- `common.h`: Structures and defines used in almost every file
- `utils.h/.c`: Small utility functions used by multiple files
- `parse.h/.c`: Functions for parsing prompt input
- `exec.h/.c`: Functions relating to actual command execution
- `builtins.h/.c`: Built-in shell function (e.g. `cd`, `exit`, etc.)

### AI Declaration
We used an AI assistant for:
- Explanations of C and POSIX concepts
- Design approach discussions (e.g. if a tokeniser was worth implementing)
- Code review and debugging
- Example code snippets (mostly in pseudocode)
- Generating large enum or define lists (e.g. color codes)