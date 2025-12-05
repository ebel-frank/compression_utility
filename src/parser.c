#include "../include/parser.h"

void parseInput(int argc, char **argv, bool *processed, bool *helpFlag, bool *versionFlag,
                bool *verboseFlag, bool *commandFlag, bool *hasError, bool *wrap, char **file, bool *stats)
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
        else if (!(*commandFlag) && ((strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)))
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
            printf("about to unwrap\n");
            *wrap = false;
            *commandFlag = true;
            processed[i] = true;
        }
        else if (strcmp(argv[i], "stats") == 0)
        {
            printf("getting stats\n");
            *stats = true;
            *wrap = false;
            *commandFlag = false;
            processed[i] = true;
        }
        else if (argv[i][0] != '-')
        {
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

static void generateOutputFile(char *file, char **outFile)
{
    size_t file_len = strlen(file);

    // Find the last occurrence of '.' to separate the path/name from the extension
    char *dot_ptr = strrchr(file, '.');

    size_t prefix_len = (dot_ptr != NULL) ? (size_t)(dot_ptr - file) : file_len;
    size_t output_len = prefix_len + strlen(GFW_EXTENSION) + 1;

    // Allocate the memory. Check for failure.
    char *output = (char *)malloc(output_len);
    if (output == NULL)
    {
        perror("Failed to allocate memory for output file name");
        *outFile = NULL;
        return;
    }

    // 4. Copy the prefix
    memcpy(output, file, prefix_len);

    // 5. Append the extension
    output[prefix_len] = '\0';     // Null-terminate the prefix
    strcat(output, GFW_EXTENSION); // Append ".gfw"

    *outFile = output;
}

void execute(bool wrap, char *file)
{
    // Get the output file path
    char *outputFile;
    if (wrap)
    {
        generateOutputFile(file, &outputFile);
    }
    else if (!wrap)
    {
        outputFile = "decompressed.bmp";
    }
    if (outputFile == NULL)
    {
        exit(EXIT_FAILURE);
    }
    printf("Inputfile: %s\n", file);
    printf("Outputfile: %s\n", outputFile);

    // Create the pipes
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1)
    {
        perror("gfwrap error: ");
        exit(EXIT_FAILURE);
    }

    // Create the child process
    pid_t pid = fork();

    if (pid < 0)
    {
        // Handle fork error
        perror("gfwrap error: ");
        exit(EXIT_FAILURE);
    }
    // Child proces
    else if (pid == 0)
    {
        // Child don't need to read from pipe
        close(pipe_fds[0]);

        // int count = 0;
        // while (true)
        // {
        //     sleep(2);
        //     write(pipe_fds[1], &count, sizeof(count));
        //     if (count == 100)
        //     {
        //         break;
        //     }
        //     count += 10;
        // }

        if (wrap)
        {
            generateHuffman(file, outputFile, pipe_fds[1]);
        }
        else
        {
            decompress(file, outputFile, pipe_fds[1]);
        }

        // clean up
        close(pipe_fds[1]);
        exit(EXIT_SUCCESS);
    }
    // Parent proces
    else
    {
        // Parent don't need to write to pipe
        close(pipe_fds[1]);
        int progress_percent = 0;

        do
        {
            if (wrap)
            {
                printf("\rCompressing: %d%%", progress_percent);
            }
            else
            {
                printf("\rDecompressing: %d%%", progress_percent);
            }
            fflush(stdout);
        } while (read(pipe_fds[0], &progress_percent, sizeof(progress_percent)) > 0);

        printf("\n");

        // Clean up
        close(pipe_fds[0]);

        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("gfwrap error: ");
        }
        else if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
        {
            printf("Completed successfully.\n");
            printf("\nTo get statistics; run 'gfwrap stats %s.gfw'.\n", outputFile);
        }
        else
        {
            printf("Failed.\n");
        }
    }

    if (wrap)
    {
        free(outputFile);
    }
}