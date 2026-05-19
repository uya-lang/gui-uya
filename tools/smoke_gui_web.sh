#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
PORT="${PORT:-0}"
VENV_DIR="${VENV_DIR:-$BUILD_DIR/.web-smoke-venv}"
SMOKE_ARGS="${SMOKE_ARGS:---backend web --demo dashboard --max-frames 3 --screenshot /tmp/last_frame.png}"
SMOKE_TIMEOUT_MS="${SMOKE_TIMEOUT_MS:-30000}"
PORT_FILE="$(mktemp)"

cleanup() {
    kill "$SERVER_PID" >/dev/null 2>&1 || true
    rm -f "$PORT_FILE"
}

mkdir -p "$BUILD_DIR"

if [ ! -x "$VENV_DIR/bin/python" ]; then
    python3 -m venv "$VENV_DIR"
fi

"$VENV_DIR/bin/python" -m pip install --quiet --upgrade pip
"$VENV_DIR/bin/python" -m pip install --quiet pyppeteer

python3 - "$PORT" "$BUILD_DIR" "$PORT_FILE" >/tmp/uya_gui_web_smoke_server.log 2>&1 <<'PY' &
import functools
import http.server
import pathlib
import socketserver
import sys

requested_port = int(sys.argv[1])
build_dir = sys.argv[2]
port_file = pathlib.Path(sys.argv[3])
handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=build_dir)

with socketserver.TCPServer(("127.0.0.1", requested_port), handler) as httpd:
    port_file.write_text(str(httpd.server_address[1]), encoding="utf-8")
    httpd.serve_forever()
PY
SERVER_PID=$!
trap cleanup EXIT

for _ in $(seq 1 50); do
    if [ -s "$PORT_FILE" ]; then
        PORT="$(cat "$PORT_FILE")"
        break
    fi
    if ! kill -0 "$SERVER_PID" >/dev/null 2>&1; then
        cat /tmp/uya_gui_web_smoke_server.log >&2
        exit 1
    fi
    sleep 0.1
done

if [ ! -s "$PORT_FILE" ]; then
    cat /tmp/uya_gui_web_smoke_server.log >&2
    exit 1
fi

SMOKE_URL="$("$VENV_DIR/bin/python" -c 'import sys, urllib.parse; print("http://127.0.0.1:%s/index.html?args=%s" % (sys.argv[1], urllib.parse.quote(sys.argv[2])))' "$PORT" "$SMOKE_ARGS")"

"$VENV_DIR/bin/python" "$ROOT_DIR/tools/smoke_gui_web.py" \
    --url "$SMOKE_URL" \
    --screenshot-path "/tmp/last_frame.png" \
    --timeout-ms "$SMOKE_TIMEOUT_MS"
