#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
WORKER_DIR="${WORKER_DIR:-$ROOT_DIR/cloudflare/ddz-openai-proxy}"
WRANGLER_BIN="${WRANGLER_BIN:-wrangler}"
TARGET_ENV="${1:-production}"

usage() {
    cat <<'EOF'
Usage: bash tools/push_ddz_openai_proxy_secrets.sh [production|dev]

Reads Cloudflare Worker secrets from:
1. UYA_OPENAI_CONFIG_FILE
2. ./.uya_openai.env
3. ./openai.env

Required:
  UPSTREAM_OPENAI_API_KEY

Optional:
  CLIENT_BEARER_TOKEN
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

CONFIG_FILE=""
if [ -n "${UYA_OPENAI_CONFIG_FILE:-}" ]; then
    CONFIG_FILE="${UYA_OPENAI_CONFIG_FILE}"
elif [ -f "$ROOT_DIR/.uya_openai.env" ]; then
    CONFIG_FILE="$ROOT_DIR/.uya_openai.env"
elif [ -f "$ROOT_DIR/openai.env" ]; then
    CONFIG_FILE="$ROOT_DIR/openai.env"
fi

if [ -n "$CONFIG_FILE" ]; then
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "error: config file not found: $CONFIG_FILE" >&2
        exit 2
    fi
    set -a
    # shellcheck disable=SC1090
    . "$CONFIG_FILE"
    set +a
    echo "info: loaded Worker secret source: $CONFIG_FILE" >&2
fi

UPSTREAM_KEY="${UPSTREAM_OPENAI_API_KEY:-${OPENAI_API_KEY:-}}"
CLIENT_TOKEN="${CLIENT_BEARER_TOKEN:-}"

if [ -z "$UPSTREAM_KEY" ]; then
    echo "error: missing UPSTREAM_OPENAI_API_KEY. Set it directly or provide OPENAI_API_KEY in your config file." >&2
    exit 2
fi

(
    cd "$WORKER_DIR"
    printf '%s' "$UPSTREAM_KEY" | "$WRANGLER_BIN" secret put UPSTREAM_OPENAI_API_KEY "${WRANGLER_ENV_ARGS[@]}"
    if [ -n "$CLIENT_TOKEN" ]; then
        printf '%s' "$CLIENT_TOKEN" | "$WRANGLER_BIN" secret put CLIENT_BEARER_TOKEN "${WRANGLER_ENV_ARGS[@]}"
    else
        echo "info: CLIENT_BEARER_TOKEN is empty; skipped optional client bearer secret." >&2
    fi
)

cat <<EOF
Cloudflare Worker secrets updated for: $TARGET_LABEL
- UPSTREAM_OPENAI_API_KEY: set
- CLIENT_BEARER_TOKEN: $( [ -n "$CLIENT_TOKEN" ] && printf 'set' || printf 'skipped' )
EOF
