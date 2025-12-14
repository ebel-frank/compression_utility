#!/usr/bin/env bash
set -euo pipefail

# Simple test harness for gfwrap
# - Builds the project
# - Runs compression on a sample BMP
# - Verifies .gfw was created
# - Decompresses and verifies the decompressed file matches original
# - Tests help/version flags and empty-file error handling

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP_DIR="$ROOT_DIR/tests/tmp"

mkdir -p "$TMP_DIR"
cd "$ROOT_DIR"

echo "Building project..."
make

SAMPLE_SRC="data/w3c_home.bmp"
if [ ! -f "$SAMPLE_SRC" ]; then
  echo "Sample file $SAMPLE_SRC not found. Please add a sample BMP to data/." >&2
  exit 1
fi

cp "$SAMPLE_SRC" "$TMP_DIR/sample.bmp"

# If valgrind is available, run a quick memory check on compress+decompress
if command -v valgrind >/dev/null 2>&1; then
  echo "Running valgrind memory checks..."
  # Use a copied sample to avoid interfering with the main test outputs
  cp "$TMP_DIR/sample.bmp" "$TMP_DIR/sample_valgrind.bmp"
  set +e
  valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
    --error-exitcode=4 --log-file="$TMP_DIR/valgrind-wrap.log" \
    ./bin/gfwrap wrap "$TMP_DIR/sample_valgrind.bmp"
  vrc=$?
  if [ $vrc -ne 0 ]; then
    echo "Valgrind detected errors during wrap (rc=$vrc). See $TMP_DIR/valgrind-wrap.log" >&2
    cat "$TMP_DIR/valgrind-wrap.log" || true
    exit 1
  fi

  valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
    --error-exitcode=4 --log-file="$TMP_DIR/valgrind-unwrap.log" \
    ./bin/gfwrap unwrap "$TMP_DIR/sample_valgrind.gfw"
  vrc=$?
  if [ $vrc -ne 0 ]; then
    echo "Valgrind detected errors during unwrap (rc=$vrc). See $TMP_DIR/valgrind-unwrap.log" >&2
    cat "$TMP_DIR/valgrind-unwrap.log" || true
    exit 1
  fi
  set -e
  # Clean valgrind run artifacts
  rm -f "$TMP_DIR/sample_valgrind.bmp" "$TMP_DIR/sample_valgrind.gfw"
  echo "Valgrind checks passed. Logs: $TMP_DIR/valgrind-*.log"
else
  echo "Valgrind not found; skipping memory checks." 
fi

echo "Compressing sample..."
./bin/gfwrap wrap "$TMP_DIR/sample.bmp"

OUT_GFW="$TMP_DIR/sample.gfw"

if [ ! -f "$OUT_GFW" ]; then
  echo "Expected output file $OUT_GFW not found" >&2
  exit 1
fi

echo "Testing round-trip decompression..."
cp "$TMP_DIR/sample.bmp" "$TMP_DIR/sample_orig.bmp"
rm -f "$TMP_DIR/sample.bmp"

./bin/gfwrap unwrap "$OUT_GFW"

DECOMP="$TMP_DIR/sample.bmp"
if [ ! -f "$DECOMP" ]; then
  echo "Decompressed file not created" >&2
  exit 1
fi

echo "Comparing original and decompressed files..."
if ! command -v sha256sum >/dev/null 2>&1; then
  echo "sha256sum not found, using cmp instead"
  cmp --silent "$TMP_DIR/sample_orig.bmp" "$DECOMP" || { echo "Files differ" >&2; exit 1; }
else
  orig_sum=$(sha256sum "$TMP_DIR/sample_orig.bmp" | awk '{print $1}')
  decomp_sum=$(sha256sum "$DECOMP" | awk '{print $1}')
  if [ "$orig_sum" != "$decomp_sum" ]; then
    echo "Checksum mismatch: $orig_sum != $decomp_sum" >&2
    exit 1
  fi
fi

echo "Help and version flags..."
./bin/gfwrap --help >/dev/null
./bin/gfwrap --version >/dev/null

echo "Testing empty-file handling..."
touch "$TMP_DIR/empty.bmp"
set +e
./bin/gfwrap wrap "$TMP_DIR/empty.bmp"
rc=$?
set -e
if [ $rc -eq 0 ]; then
  echo "Expected non-zero exit when compressing empty file" >&2
  exit 1
fi

echo
echo "All tests passed. Temporary files in: $TMP_DIR"
exit 0
