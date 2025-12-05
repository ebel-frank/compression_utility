#ifndef STATS_H
#define STATS_H

#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

void format_size(long bytes, char *buffer);
long get_file_size(const char *filename);
void printStats(char *fileName);

#endif