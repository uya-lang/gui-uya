#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WORKER_DIR="${WORKER_DIR:-$ROOT_DIR/cloudflare/ddz-openai-proxy}"
WRANGLER_BIN="${WRANGLER_BIN:-wrangler}"
TARGET_ENV="${1:-production}"
OPENAI_CONFIG_FILE="${UYA_OPENAI_CONFIG_FILE:-$ROOT_DIR/.uya_openai.env}"

usage() {
    cat <<'EOF'
Usage: bash tools/deploy_ddz_openai_proxy.sh [production|dev]

Examples:
  bash tools/deploy_ddz_openai_proxy.sh dev
  bash tools/deploy_ddz_openai_proxy.sh production
EOF
}

if [ "${TARGET_ENV}" = "-h" ] || [ "${TARGET_ENV}" = "--help" ]; then
    usage
    exit 0
fi

if ! command -v "$WRANGLER_BIN" >/dev/null 2>&1; then
    echo "error: wrangler was not found on PATH." >&2
    exit 2
fi

update_local_openai_config() {
    local worker_url="$1"
    local target_file="$OPENAI_CONFIG_FILE"
    local client_token="${CLIENT_BEARER_TOKEN:-}"

    python3 - "$target_file" "$worker_url" "$client_token" <<'PY'
import pathlib
import sys

target = pathlib.Path(sys.argv[1])
worker_url = sys.argv[2].strip()
client_token = sys.argv[3].strip()

existing = {}
order = []

if target.exists():
    for raw_line in target.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()
        if key not in existing:
            order.append(key)
        existing[key] = value

updates = {
    "UYA_DDZ_USE_OPENAI": "1",
    "OPENAI_BASE_URL": worker_url,
    "OPENAI_API_PATH": "/ddz/decision",
}
if client_token:
    updates["OPENAI_API_KEY"] = client_token

for key, value in updates.items():
    if key not in existing:
        order.append(key)
    existing[key] = value

target.parent.mkdir(parents=True, exist_ok=True)
with target.open("w", encoding="utf-8") as fh:
    for key in order:
        fh.write(f"{key}={existing[key]}\n")
PY
}

declare -a WRANGLER_ENV_ARGS=()
TARGET_LABEL="production"
case "$TARGET_ENV" in
    production|prod)
        ;;
    dev|development)
        WRANGLER_ENV_ARGS+=(--env dev)
        TARGET_LABEL="dev"
        ;;
    *)
        echo "error: unsupported target environment: $TARGET_ENV" >&2
        usage >&2
        exit 2
        ;;
esac

deploy_output="$(
    cd "$WORKER_DIR"
    "$WRANGLER_BIN" deploy "${WRANGLER_ENV_ARGS[@]}" 2>&1
)"

printf '%s\n' "$deploy_output"

worker_url="$(printf '%s\n' "$deploy_output" | grep -Eo 'https://[^[:space:])"]+' | head -n 1 || true)"

if [ -n "$worker_url" ]; then
    update_local_openai_config "$worker_url"
    echo "Local OpenAI config updated: $OPENAI_CONFIG_FILE" >&2
fi

cat <<EOF
Cloudflare Worker deploy finished for: $TARGET_LABEL
Use the URL printed above as:
- OPENAI_BASE_URL for the native simulator
- openai_base_url for the web page query/localStorage
Keep OPENAI_API_PATH / openai_api_path at /ddz/decision.
If CLIENT_BEARER_TOKEN is configured on the Worker, use the same value as the client OPENAI_API_KEY / openai_api_key.
EOF
