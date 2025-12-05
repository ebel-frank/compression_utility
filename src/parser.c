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
            *wrap = false;
            *commandFlag = true;
            processed[i] = true;
        }
        else if (strcmp(argv[i], "stats") == 0)
        {
            *stats = true;
            *wrap = false;
            *commandFlag = true;
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

// Helper: Convert bytes to human readable string
void format_size(long bytes, char *buffer)
{
    if (bytes >= 1024 * 1024)
        sprintf(buffer, "%.2f MB", bytes / (1024.0 * 1024.0));
    else if (bytes >= 1024)
        sprintf(buffer, "%.2f KB", bytes / 1024.0);
    else
        sprintf(buffer, "%ld B", bytes);
}

// Helper: Get file size
long get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1;
}

void printStats(char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        printf("Error: Could not open file %s\n", fileName);
        return;
    }

    long original_size_bytes;
    double time_taken;
    int width, height;

    // 1. Read the Header to get stored stats
    if (fscanf(file, "%ld %lf %d %d", &original_size_bytes, &time_taken, &width, &height) != 4)
    {
        printf("Error: Invalid file format.\n");
        fclose(file);
        return;
    }
    fclose(file);

    // 2. Get Compressed Size (Actual size of the .gfw file)
    long compressed_size_bytes = get_file_size(fileName);

    // 3. Calculate Ratio
    // Ratio = (1 - (compressed / original)) * 100
    float ratio = 100.0 * (1.0 - ((float)compressed_size_bytes / (float)original_size_bytes));
    if (ratio < 0)
        ratio = 0; // handle edge case if compressed is larger

    // 4. Format Output
    char orig_str[20], comp_str[20];
    format_size(original_size_bytes, orig_str);
    format_size(compressed_size_bytes, comp_str);

    printf("Compression Ratio: %.0f%%\n", ratio);
    printf("Original Size:     %s\n", orig_str);
    printf("Compressed Size:   %s\n", comp_str);
    printf("Time Elapsed:      %.2fs\n", time_taken);
}

void execute(bool stats, bool wrap, char *file)
{

    if (stats)
    {
        printf("Starting");
        printStats(file);
        exit(EXIT_SUCCESS);
    }

    // Get the output file path
    char *outputFile;
    outputFile = NULL;
    if (wrap)
    {
        generateOutputFile(file, &outputFile);
        if (outputFile == NULL)
        {
            exit(EXIT_FAILURE);
        }
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

        if (wrap && compress(file, outputFile, pipe_fds[1]) != EXIT_FAILURE)
        {
            close(pipe_fds[1]);
            exit(EXIT_FAILURE);
        }
        else if (decompress(file, pipe_fds[1]) == EXIT_FAILURE)
        {
            close(pipe_fds[1]);
            exit(EXIT_FAILURE);
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

        while (read(pipe_fds[0], &progress_percent, sizeof(progress_percent)) > 0)
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
        }

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
            printf("\nCompleted successfully.\n");
            printf("\nTo get statistics; run 'gfwrap stats %s'.\n", outputFile);
        }
        // else
        // {
        //     printf("Failed.\n");
        // }
    }

    free(outputFile);
}