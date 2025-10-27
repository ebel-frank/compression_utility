# Compression Utility - gfwrap

A lightweight, efficient image compression utility for Linux systems similar to gzip. This tool provides fast compression and decompression capabilities. It is able to take an image file and compress it using Huffman coding. It can also decompress the image back to its original. One of the features is to generate statistics.

## Features
### Functional Requirements

-	Input/Output handling – Read and write image files in simple format
-	Compression algorithm – Implement Huffman or LZ77 (or both, with a flag to choose)
-	Decompression algorithm - Accurately restore the original image from the compressed data.
-	File Format Definition - Define your own .huff or .lz77 compressed file format (with header + data).
-	CLI Interface - TBD
-	Statistics Report - Display compression ratio, original size, compressed size, time, etc.
-	Documentation - User manual + design doc describing your encoding format and algorithms.


### Non-functional Requirements

-	Efficient memory management (no leaks, good pointer discipline).
-	Modular C/C++ design (headers, source files, reusable data structures).
-	Clean build (Makefile or CMake).
-	Clear algorithm separation (I/O vs compression logic).
-	Cross-platform compatibility (runs on Linux/WSL).

## Installation

### Prerequisites
TBD

## Usage
TBD

## Performance
TBD

## Examples
TBD

## Troubleshooting
TBD

## License
TBD
