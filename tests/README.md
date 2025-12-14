# Tests for gfwrap

Run the test harness to build the project and execute a set of basic tests (compression, decompression, flags, and empty-file handling):

```bash
cd /path/to/gfwrap
bash tests/run_all.sh
```

The script uses `data/w3c_home.bmp` as a sample input. It creates temporary files under `tests/tmp/`.
