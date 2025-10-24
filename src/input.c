/*
 * input.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include <stdbool.h> // For bool type (true/false in C99 and later)
#define INITIAL_BUFFER_SIZE 1024
#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"

/*
 * read_line - Read a line of input from stdin
 *
 * Implementation uses a dynamically growing buffer. Starts with
 * INITIAL_BUFFER_SIZE and doubles when needed.
 */
char *read_line(void)
{
    int bufsize = INITIAL_BUFFER_SIZE;
    char *buffer = malloc(sizeof(char) * bufsize);
    if (!buffer)
    {
        fprintf(stderr, "qiriro-shell: memory allocation failed");
        return NULL;
    }

    int position = 0;
    while (true)
    {
        int c = getchar();
        if (c == EOF)
        {
            free(buffer);
            return NULL; /* Signal EOF to caller */
        }
        else if (c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = (char)c;
        }
        position++;

        /* Grow buffer if needed */
        if (position >= bufsize)
        {
            bufsize *= 2; /* Double the size */

            /*
             * Important: As discussed in the lecture, always assign realloc() to a temporary pointer (e.g., new_buffer)
             * before overwriting the original pointer. If realloc() fails, it returns NULL
             * without freeing the original memory block. Using a temporary variable ensures
             * you don’t lose the original pointer, which would otherwise result in a memory leak.
             */

            // Attempt to grow the buffer
            char *new_buffer = realloc(buffer, bufsize);
            if (new_buffer == NULL)
            {
                /*
                 * realloc() failed:
                 * - The original 'buffer' is still valid and must be freed to avoid a memory leak.
                 * - Returning NULL signals the caller that allocation failed.
                 */
                fprintf(stderr, "ERROR: qiriro-shell- memory allocation failed\n");
                free(buffer);
                return NULL;
            }
            /* realloc() succeeded:
             * - 'new_buffer' now points to a valid block of size 'bufsize'
             * - Safe to overwrite the original 'buffer' pointer
             */
            buffer = new_buffer;
        }
    }
}

/*
 * split_line - Splits a line of text into an array of tokens (words).
 *
 * This function takes a string (`line`) and breaks it into a sequence of
 * tokens using a set of delimiters (like space, tab, newline). It returns
 * a dynamically allocated array of strings (char **), where each string is a token.
 *
 * The implementation uses strtok(), which modifies the input `line` by inserting
 * null characters ('\0') to terminate each token. It's documentation can be found here:
 *  https://en.cppreference.com/w/c/string/byte/strtok
 *
 * It also use strings in C and use concepts that we have already explained in the lecture on strings.
 * if you need a refresher, please refer to the lecture notes.
 *
 * The array of tokens is dynamically grown as needed to accommodate any number of tokens.
 *
 * @param line The null-terminated string to be tokenized. This string will be modified.
 * @return A dynamically allocated, NULL-terminated array of character pointers (char **).
 *         Each pointer in the array points to a token within the original modified string.
 *         The caller is responsible for freeing the memory allocated for this array
 *         when it is no longer needed.
 *
 * Note: This function return a pointer to a pointer (char **). As explained in the lecture, this is because:
 *      - A single string in C is represented as a `char *` (a pointer to its first character).
 *      - We need an array of these strings to hold all the tokens.
 *      - An array of `char *` has the type `char *[]`.
 *      - When we pass an array to a function or return it, it "decays" into a pointer to its first element.
 *      - The first element of our `char *[]` array is a `char *`.
 *      - Therefore, a pointer to the first element is a `char **`.
 *      - This `char **` points to the beginning of a block of memory where each entry is a `char *` pointing to a token. The array is terminated by a NULL pointer.
 */
char **split_line(char *line)
{
    int bufsize = TOKEN_BUFFER_SIZE;
    char **tokens = malloc(bufsize * sizeof(char *));
    if (!tokens)
    {
        fprintf(stderr, "qiriro-shell: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int position = 0;
    char *token = strtok(line, TOKEN_DELIMITERS);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        /* Grow token array if needed */
        if (position >= bufsize)
        {
            bufsize += TOKEN_BUFFER_SIZE;
            char **new_tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!new_tokens)
            {
                free(tokens);
                fprintf(stderr, "qiriro-shell: memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }

        token = strtok(NULL, TOKEN_DELIMITERS);
    }
    tokens[position] = NULL; /* NULL-terminate the array */
    return tokens;
}