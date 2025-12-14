# Compression Utility - gfwrap

A lightweight, efficient image compression utility for Linux systems similar to gzip. This tool provides fast compression and decompression capabilities. It is able to take an image file and compress it using Huffman coding. It can also decompress the image back to its original. One of the features is to generate statistics.

## Features
### Functional Requirements

-	Input/Output handling – Read and write image files in simple format
-	Compression algorithm – Implemented Huffman coding
-	Decompression algorithm - Accurately restore the original image from the compressed data.
-	File Format Definition - Define your own .huff or .lz77 compressed file format (with header + data).
-	CLI Interface
-	Verbose Report - Display compression ratio, original size, compressed size, time, etc.
-	Documentation - User manual + design doc describing your encoding format and algorithms.


### Non-functional Requirements

-	Efficient memory management (no leaks, good pointer discipline).
-	Modular C/C++ design (headers, source files, reusable data structures).
-	Clean build (Makefile or CMake).
-	Clear algorithm separation (I/O vs compression logic).
-	Cross-platform compatibility (runs on Linux/WSL).

## Overview
- **Build system:** `Makefile` (POSIX `cc`).
- **CLI:** `gfwrap wrap <input>` and `gfwrap unwrap <input.gfw>`; see `--help`.

**What this repo contains**
- Source: `src/`
- Headers: `include/`
- Binary output: `bin/gfwrap` (after build)
- Tests: `tests/run_all.sh`

**Build (local)**
 - Ensure you have a POSIX C compiler (`cc`/`gcc`) installed.
 - From the project root run:

```bash
make
```

The build places the executable at `bin/gfwrap`.

## Usage
- Compress a file:

```bash
./bin/gfwrap wrap path/to/input.bmp
```

- Decompress a `.gfw` file (output file will be written next to it):

```bash
./bin/gfwrap unwrap path/to/input.gfw
```

- Show help / version:

```bash
./bin/gfwrap --help
./bin/gfwrap --version
```

## Performance
- Compressing files that are already compressed (for example `PNG`/`JPEG`) may increase their size. See `Known_Issues.md` for details and recommendations.

## Examples
TBD

## Testing
- A simple test harness is provided at `tests/run_all.sh`. It will:
	- Build the project
	- Compress a sample file (`data/w3c_home.bmp`)
	- Verify the `.gfw` files are created
	- Decompress and compare the result with the original
	- Run `valgrind` memory checks (if `valgrind` is installed)
	- Check handling of empty files and CLI flags

Run the test harness:

```bash
chmod +x tests/run_all.sh
bash tests/run_all.sh
```

You can also run the `valgrind` checks manually:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./bin/gfwrap wrap tests/tmp/sample.bmp
```

- A GitHub Actions workflow is configured to run on `workflow_dispatch` (manual trigger). It builds the code, runs tests, and uploads the `bin/gfwrap` binary as an artifact.

## License
- See `LICENSE` in the repository root.




