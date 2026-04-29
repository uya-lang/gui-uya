#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


HEADER_RE = re.compile(r"^Phase5 benchmark/report \((?P<iterations>\d+) iterations\)$")
METRIC_RE = re.compile(r"^(?P<name>.+?)\s*:\s*(?P<value>\d+)\s+(?P<unit>[A-Za-z0-9]+)$")


def parse_report(report_path: Path) -> dict:
    iterations = None
    metrics: dict[str, dict[str, int | str]] = {}

    for raw_line in report_path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line:
            continue

        header_match = HEADER_RE.match(line)
        if header_match:
            iterations = int(header_match.group("iterations"))
            continue

        metric_match = METRIC_RE.match(line)
        if metric_match:
            name = metric_match.group("name").rstrip()
            metrics[name] = {
                "value": int(metric_match.group("value")),
                "unit": metric_match.group("unit"),
            }

    if iterations is None:
        raise ValueError(f"missing benchmark header in {report_path}")
    if not metrics:
        raise ValueError(f"missing benchmark metrics in {report_path}")

    return {
        "iterations": iterations,
        "metrics": metrics,
    }


def write_json(out_path: Path, payload: dict) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def verify(parsed: dict, baseline_path: Path) -> list[str]:
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
    failures: list[str] = []

    baseline_iterations = baseline.get("iterations")
    if baseline_iterations is not None and parsed["iterations"] != baseline_iterations:
        failures.append(
            f"iterations mismatch: got {parsed['iterations']}, expected {baseline_iterations}"
        )

    baseline_metrics = baseline.get("metrics", {})
    parsed_metrics = parsed["metrics"]
    for name, rule in baseline_metrics.items():
        if name not in parsed_metrics:
            failures.append(f"missing metric: {name}")
            continue

        actual = parsed_metrics[name]
        expected_unit = rule["unit"]
        if actual["unit"] != expected_unit:
            failures.append(
                f"{name}: unit mismatch, got {actual['unit']}, expected {expected_unit}"
            )
            continue

        max_allowed = rule["max_allowed"]
        if actual["value"] > max_allowed:
            failures.append(
                f"{name}: got {actual['value']}{actual['unit']}, max allowed {max_allowed}{expected_unit}"
            )

    for invariant in baseline.get("invariants", []):
        if invariant.get("type") != "lte":
            failures.append(f"unsupported invariant type: {invariant.get('type')}")
            continue

        left_name = invariant["left"]
        right_name = invariant["right"]
        left = parsed_metrics.get(left_name)
        right = parsed_metrics.get(right_name)
        if left is None or right is None:
            failures.append(f"invariant missing metric: {left_name} <= {right_name}")
            continue
        if left["unit"] != right["unit"]:
            failures.append(
                f"invariant unit mismatch: {left_name}({left['unit']}) <= {right_name}({right['unit']})"
            )
            continue
        if left["value"] > right["value"]:
            failures.append(
                f"invariant failed: {left_name}={left['value']}{left['unit']} > "
                f"{right_name}={right['value']}{right['unit']}"
            )

    return failures


def main() -> int:
    parser = argparse.ArgumentParser(description="Parse and verify UyaGUI benchmark output.")
    parser.add_argument("--report", required=True, type=Path, help="Path to build/phase5_bench.txt")
    parser.add_argument("--baseline", type=Path, help="Path to baseline JSON with thresholds")
    parser.add_argument("--json-out", type=Path, help="Write parsed benchmark JSON to this path")
    parser.add_argument("--verify", action="store_true", help="Verify parsed metrics against --baseline")
    args = parser.parse_args()

    parsed = parse_report(args.report)
    payload = {
        "report": str(args.report),
        "iterations": parsed["iterations"],
        "metrics": parsed["metrics"],
    }

    if args.json_out:
        write_json(args.json_out, payload)

    should_verify = args.verify or args.baseline is not None
    if not should_verify:
        print(
            f"[bench-json] parsed {len(parsed['metrics'])} metrics from {args.report}",
            file=sys.stderr,
        )
        return 0

    if args.baseline is None:
        raise SystemExit("--verify requires --baseline")

    failures = verify(parsed, args.baseline)
    if failures:
        for failure in failures:
            print(f"[bench-verify] {failure}", file=sys.stderr)
        return 1

    print(
        f"[bench-verify] ok: {len(parsed['metrics'])} metrics within "
        f"{args.baseline}",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
