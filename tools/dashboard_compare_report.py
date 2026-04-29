#!/usr/bin/env python3

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
import re
import subprocess
import sys
from pathlib import Path


UYA_PROFILE_RE = re.compile(
    r"^\[uya profiler\] frames=(?P<frames>\d+) frame\(avg/max\)=(?P<avg>\d+)/(?P<max>\d+) ms "
    r"update=(?P<update>\d+) ms render=(?P<render>\d+) ms present=(?P<present>\d+) ms$",
    re.MULTILINE,
)
LVGL_PROFILE_RE = re.compile(
    r"^\[lvgl profiler\] frames=(?P<frames>\d+) frame\(avg/max\)=(?P<avg>\d+)/(?P<max>\d+) ms "
    r"update=(?P<update>\d+) ms render=(?P<render>\d+) ms present=(?P<present>\d+) ms$",
    re.MULTILINE,
)
STARTUP_RE = re.compile(r"startup=(?P<startup_us>\d+) us")
DISPLAY_RE = re.compile(r"^\[(?P<tag>uya|lvgl)-dashboard-compare\] display=(?P<display>.+)$", re.MULTILINE)
TIME_RE = re.compile(r"max_rss_kb=(?P<rss>\d+)\s+elapsed_s=(?P<elapsed>[0-9.]+)")


def parse_size(binary: Path) -> dict:
    output = subprocess.check_output(["size", str(binary)], text=True)
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    if len(lines) < 2:
        raise RuntimeError(f"unexpected size output for {binary}: {output}")
    parts = lines[1].split()
    return {
        "text": int(parts[0]),
        "data": int(parts[1]),
        "bss": int(parts[2]),
        "dec": int(parts[3]),
        "hex": parts[4],
        "file_bytes": binary.stat().st_size,
    }


def run_capture(cmd: list[str], env: dict[str, str], log_path: Path) -> str:
    proc = subprocess.run(
        cmd,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=True,
    )
    log_path.write_text(proc.stdout, encoding="utf-8")
    return proc.stdout


def run_time(cmd: list[str], env: dict[str, str], log_path: Path) -> dict:
    helper = (
        "import os, resource, subprocess, sys, time\n"
        "start = time.perf_counter()\n"
        "proc = subprocess.run(sys.argv[1:], env=os.environ, text=True, "
        "stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)\n"
        "elapsed = time.perf_counter() - start\n"
        "usage = resource.getrusage(resource.RUSAGE_CHILDREN)\n"
        "sys.stdout.write(proc.stdout)\n"
        "if proc.stdout and not proc.stdout.endswith('\\n'):\n"
        "    sys.stdout.write('\\n')\n"
        "sys.stdout.write(f'__time__ max_rss_kb={int(usage.ru_maxrss)} elapsed_s={elapsed:.6f}\\n')\n"
    )
    proc = subprocess.run(
        [sys.executable, "-c", helper, *cmd],
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    )
    combined = proc.stdout
    if proc.stderr:
        combined = proc.stdout + ("\n" if proc.stdout and not proc.stdout.endswith("\n") else "") + proc.stderr
    log_path.write_text(combined, encoding="utf-8")
    match = TIME_RE.search(proc.stdout)
    if match is None:
        raise RuntimeError(f"missing runtime footer for {' '.join(cmd)}")
    return {
        "max_rss_kb": int(match.group("rss")),
        "elapsed_s": float(match.group("elapsed")),
    }


def parse_profile(kind: str, output: str) -> dict:
    profile_re = UYA_PROFILE_RE if kind == "uya" else LVGL_PROFILE_RE
    profile_match = profile_re.search(output)
    if profile_match is None:
        raise RuntimeError(f"missing {kind} profiler output")

    startup_match = STARTUP_RE.search(output)
    if startup_match is None:
        raise RuntimeError(f"missing {kind} startup output")

    display_match = DISPLAY_RE.search(output)
    if display_match is None:
        raise RuntimeError(f"missing {kind} display output")

    return {
        "display": display_match.group("display"),
        "startup_us": int(startup_match.group("startup_us")),
        "frames": int(profile_match.group("frames")),
        "frame_avg_ms": int(profile_match.group("avg")),
        "frame_max_ms": int(profile_match.group("max")),
        "update_ms": int(profile_match.group("update")),
        "render_ms": int(profile_match.group("render")),
        "present_ms": int(profile_match.group("present")),
    }


def ratio(lhs: float, rhs: float) -> str:
    if rhs == 0:
        return "n/a"
    return f"{lhs / rhs:.2f}x"


def build_report(payload: dict) -> str:
    uya = payload["uya"]
    lvgl = payload["lvgl"]
    meta = payload["meta"]
    lines = [
        "# UyaGUI Dashboard Compare Report",
        "",
        f"- Date: {meta['date']}",
        f"- Frames: {meta['frames']}",
        f"- LVGL mode: {'rebuild' if meta['lvgl_rebuild'] else 'retained'}",
        f"- Uya binary: `{meta['uya_bin']}`",
        f"- LVGL binary: `{meta['lvgl_bin']}`",
        "",
        "| Metric | UyaGUI | LVGL | Uya/LVGL |",
        "|---|---:|---:|---:|",
        f"| Frame avg | {uya['profile']['frame_avg_ms']} ms | {lvgl['profile']['frame_avg_ms']} ms | "
        f"{ratio(uya['profile']['frame_avg_ms'], lvgl['profile']['frame_avg_ms'])} |",
        f"| Frame max | {uya['profile']['frame_max_ms']} ms | {lvgl['profile']['frame_max_ms']} ms | "
        f"{ratio(uya['profile']['frame_max_ms'], lvgl['profile']['frame_max_ms'])} |",
        f"| Startup | {uya['profile']['startup_us']} us | {lvgl['profile']['startup_us']} us | "
        f"{ratio(uya['profile']['startup_us'], lvgl['profile']['startup_us'])} |",
        f"| Max RSS | {uya['runtime']['max_rss_kb']} KB | {lvgl['runtime']['max_rss_kb']} KB | "
        f"{ratio(uya['runtime']['max_rss_kb'], lvgl['runtime']['max_rss_kb'])} |",
        f"| ELF file size | {uya['size']['file_bytes']} B | {lvgl['size']['file_bytes']} B | "
        f"{ratio(uya['size']['file_bytes'], lvgl['size']['file_bytes'])} |",
        f"| text section | {uya['size']['text']} B | {lvgl['size']['text']} B | "
        f"{ratio(uya['size']['text'], lvgl['size']['text'])} |",
        f"| data section | {uya['size']['data']} B | {lvgl['size']['data']} B | "
        f"{ratio(uya['size']['data'], lvgl['size']['data'])} |",
        f"| bss section | {uya['size']['bss']} B | {lvgl['size']['bss']} B | "
        f"{ratio(uya['size']['bss'], lvgl['size']['bss'])} |",
        "",
        "## Captured Artifacts",
        "",
        f"- Uya screenshot: `{meta['out_dir']}/uya_dashboard.bmp`",
        f"- LVGL screenshot: `{meta['out_dir']}/lvgl_dashboard.bmp`",
        f"- Uya log: `{meta['out_dir']}/uya_dashboard.log`",
        f"- LVGL log: `{meta['out_dir']}/lvgl_dashboard.log`",
        f"- Uya RSS log: `{meta['out_dir']}/uya_dashboard_rss.log`",
        f"- LVGL RSS log: `{meta['out_dir']}/lvgl_dashboard_rss.log`",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate dashboard compare report for UyaGUI vs LVGL.")
    parser.add_argument("--uya-bin", required=True, type=Path)
    parser.add_argument("--lvgl-bin", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--frames", required=True, type=int)
    parser.add_argument("--lvgl-rebuild", required=True, type=int)
    args = parser.parse_args()

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    common_env = os.environ.copy()
    common_env.setdefault("SDL_VIDEODRIVER", "dummy")

    uya_env = common_env | {
        "UYA_DASHBOARD_FRAMES": str(args.frames),
        "UYA_DASHBOARD_CAPTURE": "1",
        "UYA_DASHBOARD_OUT": str(out_dir / "uya_dashboard.bmp"),
    }
    lvgl_env = common_env | {
        "LVGL_DASHBOARD_FRAMES": str(args.frames),
        "LVGL_DASHBOARD_REBUILD": str(args.lvgl_rebuild),
        "LVGL_DASHBOARD_CAPTURE": "1",
        "LVGL_DASHBOARD_OUT": str(out_dir / "lvgl_dashboard.bmp"),
    }

    uya_output = run_capture([str(args.uya_bin)], uya_env, out_dir / "uya_dashboard.log")
    lvgl_output = run_capture([str(args.lvgl_bin)], lvgl_env, out_dir / "lvgl_dashboard.log")

    uya_time = run_time(
        [str(args.uya_bin)],
        common_env | {
            "UYA_DASHBOARD_FRAMES": str(args.frames),
            "UYA_DASHBOARD_CAPTURE": "0",
        },
        out_dir / "uya_dashboard_rss.log",
    )
    lvgl_time = run_time(
        [str(args.lvgl_bin)],
        common_env | {
            "LVGL_DASHBOARD_FRAMES": str(args.frames),
            "LVGL_DASHBOARD_REBUILD": str(args.lvgl_rebuild),
            "LVGL_DASHBOARD_CAPTURE": "0",
        },
        out_dir / "lvgl_dashboard_rss.log",
    )

    payload = {
        "meta": {
            "date": datetime.now().astimezone().isoformat(timespec="seconds"),
            "frames": args.frames,
            "lvgl_rebuild": bool(args.lvgl_rebuild),
            "uya_bin": str(args.uya_bin),
            "lvgl_bin": str(args.lvgl_bin),
            "out_dir": str(out_dir),
        },
        "uya": {
            "profile": parse_profile("uya", uya_output),
            "runtime": uya_time,
            "size": parse_size(args.uya_bin),
        },
        "lvgl": {
            "profile": parse_profile("lvgl", lvgl_output),
            "runtime": lvgl_time,
            "size": parse_size(args.lvgl_bin),
        },
    }

    (out_dir / "dashboard_compare_report.json").write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    (out_dir / "dashboard_compare_report.md").write_text(
        build_report(payload) + "\n",
        encoding="utf-8",
    )
    print(out_dir / "dashboard_compare_report.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
