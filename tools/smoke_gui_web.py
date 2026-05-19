#!/usr/bin/env python3

import argparse
import asyncio
import json
import sys

from pyppeteer import launch
from pyppeteer.errors import TimeoutError


PNG_SIGNATURE = [137, 80, 78, 71, 13, 10, 26, 10]


async def run_smoke(url: str, screenshot_path: str, timeout_ms: int) -> int:
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
            print(json.dumps({"timeout": diag}, ensure_ascii=True))
            return 1
        result = await page.evaluate(
            """(path) => {
                const out = {
                    completed: !!(window.Module && Module.uyaGuiCompleted),
                    lastPresentAt: window.Module ? (Module.uyaGuiLastPresentAt || 0) : 0,
                    completion: window.Module ? (Module.uyaGuiCompletion || null) : null,
                    screenshotExists: false,
                    screenshotBytes: 0,
                    screenshotSig: [],
                    files: window.Module ? (Module.uyaGuiFiles || {}) : {},
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
        return 0
    finally:
        await browser.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--screenshot-path", default="/tmp/last_frame.png")
    parser.add_argument("--timeout-ms", type=int, default=30000)
    args = parser.parse_args()
    return asyncio.run(run_smoke(args.url, args.screenshot_path, args.timeout_ms))


if __name__ == "__main__":
    raise SystemExit(main())
