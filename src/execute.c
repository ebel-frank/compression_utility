/*
 * execute.c - Command execution implementation
 *
 * See execute.h for interface documentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "execute.h"
#include "builtins.h"

/*
 * launch_program - Fork and execute an external program
 *
 * This function demonstrates the fork/exec pattern:
 * 1. fork() creates a child process (copy of parent)
 * 2. Child calls execvp() to replace itself with the program
 * 3. Parent waits for child to complete
 *
 * Args:
 *   args: NULL-terminated array of arguments
 *
 * Returns:
 *   1 to continue shell execution
 */
static int launch_program(char **args)
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0)
    {
        /* Child process */
        if (execvp(args[0], args) == -1)
        {
            perror("qiriro-shell");
        }
        exit(EXIT_FAILURE); /* Only reached if execvp fails */
    }
    else if (pid < 0)
    {
        /* Fork failed */
        perror("qiriro-shell");
    }
    else
    {
        /* Parent process */
        /* Wait for child to complete. WUNTRACED returns if child stopped. */
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
 * execute_command - Main command execution dispatcher
 *
 * Checks if command is built-in, otherwise launches external program.
 */
int execute_command(char **args)
{
    /* Handle empty command */
    if (args[0] == NULL)
    {
        return 1;
    }

    /* Check if command is a built-in */
    for (int i = 0; i < num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_names[i]) == 0)
        {
            return builtin_functions[i](args);
        }
    }

    /* Not a built-in, launch external program */
    return launch_program(args);
}