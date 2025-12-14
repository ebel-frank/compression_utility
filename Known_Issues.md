**Known Issues & Limitations**

This file documents the current, known limitations of the `gfwrap` utility. Items are presented with recommended mitigations or next steps when appropriate.

- **Compressing already-compressed image formats (PNG / JPEG)**
	- Symptom: Compressing a `PNG` or `JPEG` image frequently increases file size instead of reducing it.
	- Cause: PNG and JPEG are already compressed formats (entropy-coded). Running an additional lossless Huffman encoder on top of compressed data commonly cannot find additional redundancy and the container/header overhead can make the result larger.
	- Future works: Detect common compressed formats and either:
		- Skip compression and warn the user, or
		- Offer a `--force` flag to allow compression despite likely expansion

- **Large files and memory usage**
	- Symptom: The current implementation reads the entire input file into memory for building the histogram and encoding.
	- Impact: For very large files this can increase memory pressure and may fail on systems with limited RAM.
	- Future works: Implement a streaming/histogram-first pass approach or a chunked processing mode for large inputs.

- **No explicit multi-threading / parallelism**
	- Limitation: The encoder is single-threaded. For faster throughput on multi-core machines.
    - Future works: Include parallel histogram computation and/or block-based encoding.
