/*
 * main.c
 *
 * Purpose: Implements a minimal REPL (Read-Eval-Print Loop) shell that can
 * execute external commands and basic built-in commands (exit, help).
 *
 * Stage 0 Learning Goals:
 * - Understanding the REPL pattern
 * - System calls: fork(), execvp(), wait()
 * - Process creation and execution
 * - Basic user/kernel space interaction
 *
 * Architecture:
 * - main.c: Entry point and REPL loop
 * - input.c/h: Reading and tokenizing user input
 * - execute.c/h: Executing commands (both external and built-in)
 * - builtins.c/h: Built-in command implementations
 *
 * Supported Commands:
 * - External commands: ls, echo, cat, etc. (any in PATH)
 * - Built-ins: exit, help
 *
 * Memory Safety:
 * - All allocated memory is freed before next iteration
 * - Proper error handling with perror()
 *
 * Author: Kizito Nkurikiyeyezu
 *
 * File history:
 *      Creation date: 2025-09-12
 *      Version: 1.0
 *      Changes: Initial implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "execute.h"
#include "builtins.h"

/*
 * shell_loop - Main REPL (Read-Eval-Print Loop)
 *
 * This function implements the core shell loop:
 *      1. Print prompt
 *      2. Read user input
 *      3. Parse input into tokens
 *      4. Execute the command
 *      5. Repeat until user exits
 *
 * The loop continues until execute_command returns 0 (exit command).
 */

void shell_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("qiriro >>> ");
        fflush(stdout); /* Ensure prompt is displayed immediately */

        line = read_line();
        if (line == NULL)
        {
            /* EOF received (Ctrl+D), exit gracefully */
            printf("\n");
            fprintf(stderr, "EOF received (Ctrl+D) - exiting shell gracefully\n");
            exit(EXIT_SUCCESS);
            // break;
        }

        args = split_line(line);
        status = execute_command(args);

        free(line);
        free(args);
    } while (status);
}

/*
 * main - Entry point for the shell
 *
 * Args:
 *   argc: Argument count (kept for future use -- unused in Stage 0)
 *   argv: Argument vector (kept for future use -- unused in Stage 0)
 *
 * Returns:
 *   EXIT_SUCCESS on normal termination
 */
int main(int argc, char **argv)
{
    /* Unused parameters - suppress compiler warnings */
    (void)argc;
    (void)argv;

    shell_loop();
    return EXIT_SUCCESS;
}