#include "util.h"

void printHelp()
{
    printf("Usage: gfwrap [OPTIONS]\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --version           Show version information\n");
    printf("  -o, --output <file>     Specify output file\n");
    printf("  -v, --verbose           Enable verbose mode\n");
}

void printVersion()
{
    printf("gfwrap version 1.0.0, build 456cc45\n");
}