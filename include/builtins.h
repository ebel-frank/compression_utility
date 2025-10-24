/*
 * builtins.h - Built-in command interface
 * 
 * Purpose: Defines built-in commands that are handled directly by the shell
 * rather than launching external programs.
 * 
 * Stage 0 Built-ins:
 * - exit: Terminate the shell
 * - help: Display help information
 * 
 * Design:
 * - Uses parallel arrays for names and function pointers
 * - All built-in functions follow same signature
 * - Easy to add new built-ins by extending arrays
 * 
 * Public API:
 * - builtin_names[]: Array of built-in command names
 * - builtin_functions[]: Array of function pointers
 * - num_builtins(): Returns count of built-ins
 * - Individual built-in functions (builtin_exit, builtin_help)
 */

#ifndef BUILTINS_H
#define BUILTINS_H

/*
 * Built-in function signature
 * 
 * All built-in functions take:
 *   args: NULL-terminated array of arguments (args[0] is command name)
 * 
 * And return:
 *   1 to continue shell execution
 *   0 to terminate shell
 */
typedef int (*builtin_func)(char **);

/* Built-in command declarations */
int builtin_exit(char **args);
int builtin_help(char **args);

/* Arrays mapping command names to functions */
extern char *builtin_names[];
extern builtin_func builtin_functions[];

/* Get number of built-in commands */
int num_builtins(void);

#endif /* BUILTINS_H */