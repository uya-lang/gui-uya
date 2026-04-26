#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
UYA_BIN="${UYA:-$ROOT_DIR/uya/bin/uya}"
CC_BIN="${CC:-cc}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/sim}"
OUT_C="$BUILD_DIR/gui_uya_sim.c"
OUT_BIN="$BUILD_DIR/gui_uya_sim"
OUT_GEN_O="$BUILD_DIR/gui_uya_sim.generated.o"
OUT_SDL_O="$BUILD_DIR/gui_uya_sim.sdl_host.o"
OUT_FB_O="$BUILD_DIR/gui_uya_sim.fb_host.o"
MODE="${MODE:-debug}"

mkdir -p "$BUILD_DIR"

SDL_CFLAGS_STR=""
SDL_LIBS_STR=""
if pkg-config --exists sdl2 2>/dev/null; then
    SDL_CFLAGS_STR="$(pkg-config --cflags sdl2)"
    SDL_LIBS_STR="$(pkg-config --libs sdl2)"
elif command -v sdl2-config >/dev/null 2>&1; then
    SDL_CFLAGS_STR="$(sdl2-config --cflags)"
    SDL_LIBS_STR="$(sdl2-config --libs)"
else
    echo "error: SDL2 development files were not found." >&2
    echo "hint : install libsdl2-dev (Debian/Ubuntu) or SDL2-devel (Fedora/RHEL), then rerun 'make sim-build'." >&2
    exit 2
fi

UYA_OPT="-O0"
CC_OPT="-O0"
if [ "$MODE" = "release" ]; then
    UYA_OPT="-O3"
    CC_OPT="-O2"
fi

read -r -a SDL_CFLAGS <<<"$SDL_CFLAGS_STR"
read -r -a SDL_LIBS <<<"$SDL_LIBS_STR"

"$UYA_BIN" build "$ROOT_DIR/gui/sim_main.uya" --c99 --no-split-c "$UYA_OPT" -o "$OUT_C"

HOST_C="$ROOT_DIR/gui/platform/sdl2/sdl_host.c"
FB_HOST_C="$ROOT_DIR/gui/platform/fb/fb_host.c"

# 生成的 Uya C 代码会携带大量与宿主 libc / compiler 内建相关的噪声 warning，
# 这里静默编译生成物，只保留我们手写 host glue 的真实 warning。
"$CC_BIN" -std=c99 -g "$CC_OPT" -fno-builtin -fvisibility=hidden -w \
    "${SDL_CFLAGS[@]}" \
    -c "$OUT_C" \
    -o "$OUT_GEN_O"

"$CC_BIN" -std=c99 -Wall -Wextra -pedantic -g "$CC_OPT" -fvisibility=hidden \
    "${SDL_CFLAGS[@]}" \
    -c "$HOST_C" \
    -o "$OUT_SDL_O"

"$CC_BIN" -std=c99 -Wall -Wextra -pedantic -g "$CC_OPT" -fvisibility=hidden \
    "${SDL_CFLAGS[@]}" \
    -c "$FB_HOST_C" \
    -o "$OUT_FB_O"

"$CC_BIN" -g "$CC_OPT" \
    "$OUT_GEN_O" \
    "$OUT_SDL_O" \
    "$OUT_FB_O" \
    -o "$OUT_BIN" \
    "${SDL_LIBS[@]}" \
    -ldl \
    -lm

echo "$OUT_BIN"
