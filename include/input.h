/*
 * input.h - Input reading and tokenization interface
 * 
 * Purpose: Provides functions for reading user input and splitting it into
 * tokens for command execution.
 * 
 * Public API:
 * - read_line(): Reads a line of input from stdin
 * - split_line(): Splits a line into space-delimited tokens
 * 
 * Memory Safety:
 * - read_line() returns malloc'd memory - caller must free()
 * - split_line() returns malloc'd array of tokens - caller must free()
 * - Tokens point into the original line buffer (shared ownership)
 * 
 * Error Handling:
 * - Returns NULL on EOF or allocation failure
 * - Prints error messages to stderr
 */

#ifndef INPUT_H
#define INPUT_H

/*
 * read_line - Read a line of input from stdin
 * 
 * Reads characters until newline or EOF. Dynamically allocates and grows
 * buffer as needed.
 * 
 * Returns:
 *   Pointer to malloc'd string containing the line (without newline)
 *   NULL on EOF or allocation error
 * 
 * Memory:
 *   Caller must free() the returned string
 */
char *read_line(void);

/*
 * split_line - Split a line into tokens
 * 
 * Splits input on whitespace (space, tab, newline). Uses strtok() internally,
 * which modifies the input string.
 * 
 * Args:
 *   line: The line to split (will be modified by strtok)
 * 
 * Returns:
 *   NULL-terminated array of token pointers
 *   Returns array with single NULL on empty input
 * 
 * Memory:
 *   Caller must free() the returned array (but NOT individual tokens,
 *   as they point into the original line buffer)
 */
char **split_line(char *line);

#endif /* INPUT_H */