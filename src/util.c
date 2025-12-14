#include "../include/util.h"

void printHelp()
{
    printf("Usage: gfwrap [OPTIONS] <command> <FILE>\n");
    printf("Wrap or unwrap a FILE. Uses Huffman coding algorithm\n\n");
    printf("Commands:\n");
    printf("  wrap                wrap (compress) a file\n");
    printf("  unwrap              unwrap (decompress) a file\n");
    printf("Options:\n");
    printf("  -h, --help          show this help message\n");
    printf("  -V, --verbose       verbase mode\n");
    printf("  -v, --version       show version information\n");
}

void printWrapHelp() {
    printf("Usage: gfwrap wrap [OPTIONS] <input_file>\n\n");
    printf("Options:\n");
    printf("  -V, --verbose           verbose mode\n");
}

void printUnwrapHelp() {
    printf("Usage: gfwrap unwrap [OPTIONS] <input_file>\n\n");
    printf("Options:\n");
    printf("  -V, --verbose           verbose mode\n");
}

void printVersion()
{
    printf("gfwrap version 1.2.0, build 456cc45\n");
}