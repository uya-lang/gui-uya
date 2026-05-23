#!/usr/bin/env python3

import argparse
import asyncio
import json
import sys

from pyppeteer import launch
from pyppeteer.errors import TimeoutError


PNG_SIGNATURE = [137, 80, 78, 71, 13, 10, 26, 10]
RICHTEXT_INPUT_TOKEN = "[[SMOKE_INPUT]]"
RICHTEXT_PASTE_TOKEN = "\n[[SMOKE_PASTE]]"


def parse_expect_sizes(raw_values: list[str]) -> list[int]:
    sizes: list[int] = []
    for raw in raw_values:
        for part in raw.split(","):
            part = part.strip()
            if not part:
                continue
            sizes.append(int(part))
    return sizes


async def read_richtext_export(page, copy_fn_name: str) -> str | None:
    return await page.evaluate(
        """(copyFnName) => {
            if (!window.Module || !Module[copyFnName] || !Module._malloc || !Module._free || typeof TextDecoder === 'undefined') {
              return null;
            }
            var cap = 16384;
            var ptr = Module._malloc(cap);
            try {
              var len = Module[copyFnName](ptr, cap) | 0;
              if (len <= 0) {
                return '';
              }
              var bytes = Module.HEAPU8.slice(ptr, ptr + len);
              return new TextDecoder('utf-8').decode(bytes);
            } finally {
              Module._free(ptr);
            }
        }""",
        copy_fn_name,
    )


async def wait_for_richtext_export_contains(page, copy_fn_name: str, token: str, timeout_ms: int) -> str | None:
    deadline = asyncio.get_running_loop().time() + timeout_ms / 1000.0
    last_value = None
    while asyncio.get_running_loop().time() < deadline:
        last_value = await read_richtext_export(page, copy_fn_name)
        if isinstance(last_value, str) and token in last_value:
            return last_value
        await asyncio.sleep(0.05)
    return last_value


async def run_richtext_smoke(page, timeout_ms: int) -> dict:
    await page.waitForFunction(
        "window.Module && (Module.uyaGuiLastPresentAt || 0) > 0",
        {"timeout": timeout_ms},
    )
    input_ok = await page.evaluate(
        """(token) => {
            if (!window.Module || !Module._uya_gui_web_host_richtext_insert_text || !Module._malloc || !Module._free) {
              return false;
            }
            var bytes = new TextEncoder().encode(token);
            var ptr = Module._malloc(bytes.length);
            try {
              Module.HEAPU8.set(bytes, ptr);
              return !!Module._uya_gui_web_host_richtext_insert_text(ptr, bytes.length | 0);
            } finally {
              Module._free(ptr);
            }
        }""",
        RICHTEXT_INPUT_TOKEN,
    )
    if not input_ok:
        return {"ok": False, "error": "input_command_unavailable"}

    plain_after_input = await wait_for_richtext_export_contains(
        page,
        "_uya_gui_web_host_richtext_plain_text_copy",
        RICHTEXT_INPUT_TOKEN,
        timeout_ms,
    )
    if not isinstance(plain_after_input, str) or RICHTEXT_INPUT_TOKEN not in plain_after_input:
        return {
            "ok": False,
            "error": "input_token_missing",
            "plain": plain_after_input,
        }

    paste_ok = await page.evaluate(
        """(text) => {
            if (!window.Module || !Module._uya_gui_web_host_richtext_paste_plain || !Module._malloc || !Module._free) {
              return false;
            }
            var bytes = new TextEncoder().encode(text);
            var ptr = Module._malloc(bytes.length);
            try {
              Module.HEAPU8.set(bytes, ptr);
              return !!Module._uya_gui_web_host_richtext_paste_plain(ptr, bytes.length | 0);
            } finally {
              Module._free(ptr);
            }
        }""",
        RICHTEXT_PASTE_TOKEN,
    )
    if not paste_ok:
        return {"ok": False, "error": "paste_command_unavailable"}

    final_plain = await wait_for_richtext_export_contains(
        page,
        "_uya_gui_web_host_richtext_plain_text_copy",
        "[[SMOKE_PASTE]]",
        timeout_ms,
    )
    if not isinstance(final_plain, str) or "[[SMOKE_PASTE]]" not in final_plain:
        return {
            "ok": False,
            "error": "paste_token_missing",
            "plain": final_plain,
        }

    final_html = await read_richtext_export(page, "_uya_gui_web_host_richtext_html_copy")
    if not isinstance(final_html, str):
        return {"ok": False, "error": "html_export_unavailable"}
    if RICHTEXT_INPUT_TOKEN not in final_html or "[[SMOKE_PASTE]]" not in final_html or "<p>" not in final_html:
        return {
            "ok": False,
            "error": "html_export_missing_tokens",
            "html": final_html,
        }

    richtext_state = await page.evaluate(
        """() => {
            var sink = document.getElementById('uya-gui-ime-sink');
            return {
              activeSession: window.Module && Module._uya_gui_web_host_richtext_active_session
                ? (Module._uya_gui_web_host_richtext_active_session() | 0)
                : 0,
              sinkFocused: !!(sink && document.activeElement === sink),
              sinkMode: sink ? (sink.dataset.uyaMode || '') : '',
              sinkActive: sink ? (sink.dataset.uyaRichtextActive || '') : '',
            };
        }"""
    )

    return {
        "ok": True,
        "plain": final_plain,
        "html": final_html,
        "state": richtext_state,
    }


async def run_smoke(
    url: str,
    screenshot_path: str,
    timeout_ms: int,
    expect_bitmap_ready_sizes: list[int],
    expect_bitmap_ready_at_least: int,
    expect_bitmap_requested_at_most: int | None,
    scenario: str,
) -> int:
    browser = await launch(
        headless=True,
        args=[
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--disable-dev-shm-usage",
        ],
    )
    page = await browser.newPage()
    page.on("console", lambda msg: print(f"[browser:{msg.type}] {msg.text}"))
    page.on("pageerror", lambda err: print(f"[pageerror] {err}"))
    try:
        await page.goto(url, waitUntil="networkidle0", timeout=timeout_ms)
        scenario_result = None
        if scenario == "richtext":
            scenario_result = await run_richtext_smoke(page, timeout_ms)
            if scenario_result.get("ok") is not True:
                print(json.dumps({"richtext": scenario_result}, ensure_ascii=True))
                return 1
        try:
            await page.waitForFunction(
                "window.Module && Module.uyaGuiCompleted === true",
                {"timeout": timeout_ms},
            )
        except TimeoutError:
            diag = await page.evaluate(
                """(path) => {
                    const mod = window.Module || null;
                    const out = {
                        completed: !!(mod && mod.uyaGuiCompleted),
                        lastPresentAt: mod ? (mod.uyaGuiLastPresentAt || 0) : 0,
                        completion: mod ? (mod.uyaGuiCompletion || null) : null,
                        args: mod ? (mod.arguments || []) : [],
                        status: document.getElementById('uya-gui-status') ? document.getElementById('uya-gui-status').textContent : '',
                        log: document.getElementById('uya-gui-log') ? document.getElementById('uya-gui-log').textContent : '',
                        screenshotExists: false,
                    };
                    if (typeof FS !== 'undefined' && path) {
                        try {
                            out.screenshotExists = FS.analyzePath(path).exists;
                        } catch (err) {
                            out.fsError = String(err);
                        }
                    }
                    return out;
                }""",
                screenshot_path,
            )
            print(json.dumps({"timeout": diag, "scenario": scenario_result}, ensure_ascii=True))
            return 1
        result = await page.evaluate(
            """(path) => {
                const bitmapStates = (window.Module && Module.uyaGuiBitmapFontStates) ? Module.uyaGuiBitmapFontStates : {};
                const bitmapRequested = Object.keys(bitmapStates).map((key) => Number(key)).sort((a, b) => a - b);
                const bitmapReady = bitmapRequested.filter((size) => bitmapStates[size] === 'ready');
                const out = {
                    completed: !!(window.Module && Module.uyaGuiCompleted),
                    lastPresentAt: window.Module ? (Module.uyaGuiLastPresentAt || 0) : 0,
                    completion: window.Module ? (Module.uyaGuiCompletion || null) : null,
                    screenshotExists: false,
                    screenshotBytes: 0,
                    screenshotSig: [],
                    files: window.Module ? (Module.uyaGuiFiles || {}) : {},
                    bitmapFontStates: bitmapStates,
                    bitmapFontRequested: bitmapRequested,
                    bitmapFontReady: bitmapReady,
                };
                if (typeof FS !== 'undefined' && path) {
                    try {
                        if (FS.analyzePath(path).exists) {
                            const bytes = FS.readFile(path);
                            out.screenshotExists = true;
                            out.screenshotBytes = bytes.length;
                            out.screenshotSig = Array.from(bytes.slice(0, 8));
                        }
                    } catch (err) {
                        out.fsError = String(err);
                    }
                }
                return out;
            }""",
            screenshot_path,
        )
        result["scenario"] = scenario_result
        print(json.dumps(result, ensure_ascii=True))
        if not result["completed"]:
            print("smoke failed: app did not complete", file=sys.stderr)
            return 1
        if result["lastPresentAt"] <= 0:
            print("smoke failed: no frame was presented", file=sys.stderr)
            return 1
        if not result["screenshotExists"] or result["screenshotBytes"] <= 0:
            print("smoke failed: screenshot file missing or empty", file=sys.stderr)
            return 1
        if result["screenshotSig"] != PNG_SIGNATURE:
            print("smoke failed: screenshot is not a PNG payload", file=sys.stderr)
            return 1
        if len(result["bitmapFontReady"]) < expect_bitmap_ready_at_least:
            print(
                "smoke failed: too few ready bitmap fonts "
                f"({len(result['bitmapFontReady'])} < {expect_bitmap_ready_at_least})",
                file=sys.stderr,
            )
            return 1
        for size in expect_bitmap_ready_sizes:
            if size not in result["bitmapFontReady"]:
                print(f"smoke failed: expected ready bitmap font size {size}", file=sys.stderr)
                return 1
        if (
            expect_bitmap_requested_at_most is not None
            and len(result["bitmapFontRequested"]) > expect_bitmap_requested_at_most
        ):
            print(
                "smoke failed: too many bitmap font sizes requested "
                f"({len(result['bitmapFontRequested'])} > {expect_bitmap_requested_at_most})",
                file=sys.stderr,
            )
            return 1
        return 0
    finally:
        await browser.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--screenshot-path", default="/tmp/last_frame.png")
    parser.add_argument("--timeout-ms", type=int, default=30000)
    parser.add_argument("--expect-bitmap-ready-size", action="append", default=[])
    parser.add_argument("--expect-bitmap-ready-at-least", type=int, default=0)
    parser.add_argument("--expect-bitmap-requested-at-most", type=int, default=None)
    parser.add_argument("--scenario", choices=["basic", "richtext"], default="basic")
    args = parser.parse_args()
    return asyncio.run(
        run_smoke(
            args.url,
            args.screenshot_path,
            args.timeout_ms,
            parse_expect_sizes(args.expect_bitmap_ready_size),
            args.expect_bitmap_ready_at_least,
            args.expect_bitmap_requested_at_most,
            args.scenario,
        )
    )


if __name__ == "__main__":
    raise SystemExit(main())
