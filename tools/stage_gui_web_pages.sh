#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
PAGES_DIR="${PAGES_DIR:-$ROOT_DIR/build/pages}"

require_asset() {
    local path="$1"
    if [ ! -f "$path" ]; then
        echo "error: missing web asset: $path" >&2
        exit 1
    fi
}

require_asset "$BUILD_DIR/index.html"
require_asset "$BUILD_DIR/index.js"
require_asset "$BUILD_DIR/index.wasm"
require_asset "$BUILD_DIR/index.data"

rm -rf "$PAGES_DIR"
mkdir -p "$PAGES_DIR"

for asset in "$BUILD_DIR"/index.*; do
    if [ -f "$asset" ]; then
        cp -a "$asset" "$PAGES_DIR"/
    fi
done

: > "$PAGES_DIR/.nojekyll"

echo "$PAGES_DIR"
