#include "../include/util.h"

void printHelp()
{
    printf("Usage: gfwrap\t[OPTIONS]\n\t\t<command> [<args>]\n");
    printf("Commands:\n");
    printf("  wrap                Wrap/compress a file\n");
    printf("  unwrap              Unwrap/decompress a file\n");
    printf("  stat                Show file information\n\n");
    printf("Options:\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -v, --version       Show version information\n");
}

void printWrapHelp() {
    printf("Usage: gfwrap wrap [OPTIONS] <input_file>\n\n");
    printf("Options:\n");
    printf("  -algo <algorithm>   Compression algorithm (huffman)\n");
    printf("  -v, --verbose           Enable verbose mode\n");
}

void printUnwrapHelp() {
    printf("Usage: gfwrap unwrap [OPTIONS] <input_file>\n\n");
    printf("Options:\n");
    printf("  -v, --verbose           Enable verbose mode\n");
}

void printVersion()
{
    printf("gfwrap version 1.0.0, build 456cc45\n");
}