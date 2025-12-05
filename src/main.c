/*
 * main.c
 *
 * Purpose: Implements an efficient Image compression tool
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
 * Author: Frank Ebeledike and George Ocran
 *
 * File history:
 *      Creation date: 2025-11-20
 *      Version: 1.0
 *      Changes: Initial implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/bitmap.h"
#include "../include/parser.h"
#include "../include/util.h"
#include "../include/util.h"

/*
 * main - Entry point for the compression tool
 *
 * Args:
 *   argc: Argument count (kept for future use -- unused in Stage 0)
 *   argv: Argument vector (kept for future use -- unused in Stage 0)
 *
 * Returns:
 *   EXIT_SUCCESS on normal termination
 *   EXIT_FAILURE in case of a failure
 */
int main(int argc, char **argv)
{
    bool *processed = calloc(argc, sizeof(bool));
    processed[0] = true; // Mark program name as processed

    bool helpFlag = false;
    bool versionFlag = false;
    bool commandFlag = false;
    bool hasError = false;
    bool verboseFlag = false;
    bool wrap = false;
    bool statsFlag = false;
    char *file = NULL;

    parseInput(argc, argv, processed, &helpFlag, &versionFlag, &verboseFlag,
               &commandFlag, &hasError, &wrap, &file, &statsFlag);

    printf("Help: %d, error: %d, version: %d, verbase: %d, command: %d \n", helpFlag, hasError, versionFlag, verboseFlag, commandFlag);
    // printf("Check: %d", )
    free(processed);
    // Handle flags
    if (hasError)
    {
        fprintf(stderr, "\n");
        printHelp();
        return EXIT_FAILURE;
    }
    if (!commandFlag && helpFlag)
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    if (!commandFlag && statsFlag)
    {
        printStats(file);
        return EXIT_SUCCESS;
    }
    if (!commandFlag && versionFlag)
    {
        printVersion();
        return EXIT_SUCCESS;
    }
    if (commandFlag && helpFlag)
    {
        if (wrap)
        {
            printWrapHelp();
        }
        else
        {
            printUnwrapHelp();
        }
        return EXIT_SUCCESS;
    }
    if (!file)
    {

        if (wrap)
        {
            printWrapHelp();
        }
        else
        {
            printUnwrapHelp();
        }
        return EXIT_FAILURE;
    }
    // printf("file: %s\n", file);
    execute(wrap, file);

    // if (argc != 2) {
    //     printf("Usage: %s <image_file>\n", argv[0]);
    //     return EXIT_FAILURE;
    // }
    // printf("Value of argc: %d\n", argc);
    // printf("Value of argv[0]: %s\n", argv[0]);
    // printf("Value of argv[1]: %s\n", argv[1]);
    // readImageFile(argv[1]);
    return EXIT_SUCCESS;
}