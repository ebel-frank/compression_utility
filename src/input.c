#include "../include/input.h"

void parseInput(int argc, char **argv, bool *processed, bool *helpFlag, bool *versionFlag,
                bool *verboseFlag, bool *commandFlag, bool *hasError, bool *wrap, char **file)
{
    for (int i = 0; i < argc; i++)
    {
        if (processed[i])
        {
            continue; // Skip already processed arguments
        }

        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            *helpFlag = true;
            processed[i] = true;
        }
        else if (!commandFlag && (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0))
        {
            *versionFlag = true;
            processed[i] = true;
        }
        else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-V") == 0)
        {
            *verboseFlag = true;
            processed[i] = true;
        }
        else if (strcmp(argv[i], "wrap") == 0)
        {
            *wrap = true;
            *commandFlag = true;
            processed[i] = true;
        }
        else if (strcmp(argv[i], "unwrap") == 0)
        {
            *wrap = false;
            *commandFlag = true;
            processed[i] = true;
        } else if (argv[i][0] != '-') {
            *file = argv[i];
            processed[i] = true;
        }
    }

    // Check for unprocessed arguments (unsupported options)
    for (int i = 1; i < argc; i++)
    {
        if (!processed[i])
        {
            fprintf(stderr, "Error: Unknown argument '%s'\n", argv[i]);
            *hasError = true;
        }
    }
}