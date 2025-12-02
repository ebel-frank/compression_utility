#ifndef INPUT_H
#define INPUT_H
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/huffman.h"

#define GFW_EXTENSION ".gfw"

void parseInput(int argc, char **argv, bool *processed, bool *helpFlag, bool *versionFlag,
                bool *verboseFlag, bool *commandFlag, bool *hasError, bool *wrap, char **file);

void execute(bool wrap, char *file);

#endif