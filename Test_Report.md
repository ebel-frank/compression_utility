**Test Report — gfwrap**

This document summarizes the major test cases executed against the `gfwrap` compression utility, the environment used for testing, and observed outcomes.

**Environment**
- OS: Linux
- Compiler: POSIX `cc`
- Test harness: `tests/run_all.sh`
- Valgrind (Optional): `valgrind`

**Test Cases**

- **Build**
	- Command: `make`
	- Expected: `bin/gfwrap` is produced
	- Outcome: PASS — project builds successfully (object files under `bin/obj`, final binary `bin/gfwrap`).

- **Functional compression & decompression (round-trip)**
	- Input: `data/w3c_home.bmp` (sample bitmap)
	- Commands: `./bin/gfwrap wrap tests/tmp/sample.bmp` then `./bin/gfwrap unwrap tests/tmp/sample.gfw`
	- Expected: `.gfw` created; decompressed file equals original (byte-for-byte)
	- Outcome: PASS — `.gfw` file created; decompressed output compared using `sha256sum` matched original.
	- Artifacts: `tests/tmp/sample.gfw`, `tests/tmp/sample.bmp` (decompressed output)

- **Empty-file handling**
	- Input: zero-byte file created in `tests/tmp/empty.bmp`
	- Expected: compression command returns non-zero and prints an error
	- Outcome: PASS — utility exits with non-zero status when attempting to compress empty file.

- **CLI flags (help/version)**
	- Commands: `./bin/gfwrap --help`, `./bin/gfwrap --version`
	- Outcome: PASS — both commands execute and exit successfully.

- **Memory checks (Valgrind)**
	- When `valgrind` is installed the harness runs a short compress+decompress under Valgrind:
		- `tests/tmp/valgrind-wrap.log`
		- `tests/tmp/valgrind-unwrap.log`
	- Outcome: PASS — valgrind reported zero memory leaks.


