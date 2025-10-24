/*
 * execute.h - Command execution interface
 * 
 * Purpose: Handles execution of both built-in and external commands.
 * 
 * Public API:
 * - execute_command(): Main execution dispatcher
 * 
 * Architecture:
 * - Checks if command is a built-in first
 * - Falls back to launching external command via fork/exec
 * 
 * System Calls Used:
 * - fork(): Create child process
 * - execvp(): Replace process image with external program
 * - wait(): Wait for child process to complete
 * 
 * Error Handling:
 * - Handles fork failures
 * - Handles execvp failures (command not found)
 * - Uses perror() for system call errors
 */

#ifndef EXECUTE_H
#define EXECUTE_H

/*
 * execute_command - Execute a command (built-in or external)
 * 
 * First checks if the command is a built-in (exit, help). If so, executes
 * it directly. Otherwise, forks a child process and uses execvp() to run
 * an external program.
 * 
 * Args:
 *   args: NULL-terminated array of command arguments
 *         args[0] is the command name
 *         args[1..n] are the arguments
 * 
 * Returns:
 *   1 to continue shell execution
 *   0 to terminate shell (from exit command)
 * 
 * Behavior:
 * - Empty command (args[0] == NULL) returns 1 (continue)
 * - Parent process waits for child to complete
 * - Child process replaces itself with the external program
 */
int execute_command(char **args);

#endif /* EXECUTE_H */