/*
 * builtins.c - Built-in command implementations
 *
 * See builtins.h for interface documentation.
 */

#include <stdio.h>
#include "builtins.h"

/*
 * Parallel arrays for built-in commands
 * To add a new built-in:
 * 1. Add function declaration in builtins.h
 * 2. Implement function in this file
 * 3. Add name to builtin_names[]
 * 4. Add function pointer to builtin_functions[]
 */
char *builtin_names[] = {
    "exit",
    "help"};

builtin_func builtin_functions[] = {
    &builtin_exit,
    &builtin_help};

/*
 * num_builtins - Get count of built-in commands
 */
int num_builtins(void)
{
    return sizeof(builtin_names) / sizeof(char *);
}

/*
 * builtin_exit - Exit the shell
 *
 * Args:
 *   args: Command arguments (unused)
 *
 * Returns:
 *   0 to signal shell termination
 */
int builtin_exit(char **args)
{
    (void)args; /* Unused */
    return 0;
}

/*
 * builtin_help - Display help information
 *
 * Lists available built-in commands and provides basic usage info.
 *
 * Args:
 *   args: Command arguments (unused)
 *
 * Returns:
 *   1 to continue shell execution
 */
int builtin_help(char **args)
{
    (void)args; /* Unused */

    printf("qiriro-shell - Stage 0\n");
    printf("Built-in commands:\n");

    for (int i = 0; i < num_builtins(); i++)
    {
        printf("  %s\n", builtin_names[i]);
    }

    printf("\nExternal commands:\n");
    printf("  Any command in your PATH (e.g., ls, echo, cat, pwd)\n");
    printf("  Example: ls -la\n");
    printf("  Example: echo Hello World\n\n");
    printf("Use 'man <command>' for help on external commands.\n");

    return 1;
}