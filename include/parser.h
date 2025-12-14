#ifndef INPUT_H
#define INPUT_H
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "../include/huffman.h"

#define GFW_EXTENSION ".gfw"

void parseInput(int argc, char **argv, bool *processed, bool *helpFlag, bool *versionFlag,
                bool *verboseFlag, bool *commandFlag, bool *hasError, bool *wrap, char **file);

void execute(bool stats, bool wrap, char *file);

void format_size(long bytes, char *buffer);

long get_file_size(const char *filename);

void printStats(char *fileName);

#endif