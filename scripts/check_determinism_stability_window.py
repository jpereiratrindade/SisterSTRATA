#!/usr/bin/env python3
"""Analyze determinism hash stability across a historical workflow window.

This script compares the current run deterministic hash against hashes extracted
from previous successful workflow runs on the same branch.
"""

from __future__ import annotations

import argparse
import io
import json
import os
import pathlib
import re
import sys
import urllib.error
import urllib.parse
import urllib.request
import zipfile
from dataclasses import dataclass
from typing import Iterable


HASH_RE = re.compile(r"^[0-9a-f]{64}$")
ARTIFACT_PREFIX = "determinism-state-hash-"


@dataclass
class RunHash:
    run_id: int
    run_number: int
    run_attempt: int
    html_url: str
    hash_value: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check determinism hash stability over recent successful CI runs."
    )
    parser.add_argument("--hash-root", default="determinism-hashes")
    parser.add_argument("--repo", default=os.environ.get("GITHUB_REPOSITORY", ""))
    parser.add_argument("--branch", default=os.environ.get("GITHUB_REF_NAME", "main"))
    parser.add_argument("--workflow-name", default=os.environ.get("GITHUB_WORKFLOW", "STRATA-CI"))
    parser.add_argument("--current-run-id", type=int, default=int(os.environ.get("GITHUB_RUN_ID", "0")))
    parser.add_argument("--window", type=int, default=20)
    parser.add_argument("--min-samples", type=int, default=5)
    parser.add_argument("--promotion-min-samples", type=int, default=20)
    parser.add_argument("--mode", choices=("warn", "enforce", "adaptive"), default="warn")
    parser.add_argument("--summary-path", default="build/headless/determinism/stability_window_summary.md")
    parser.add_argument("--json-path", default="build/headless/determinism/stability_window_report.json")
    parser.add_argument(
        "--promotion-ready-path",
        default="build/headless/determinism/stability_window_promotion_ready.txt",
    )
    return parser.parse_args()


def fail_or_warn(mode: str, message: str) -> int:
    if mode == "enforce":
        print(f"[determinism-stability][FAIL] {message}")
        return 1
    print(f"::warning::{message}")
    return 0


def write_promotion_ready_flag(path: pathlib.Path, value: bool | None) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if value is None:
        path.write_text("unknown\n", encoding="utf-8")
        return
    path.write_text(("1\n" if value else "0\n"), encoding="utf-8")


def write_issue_outputs(
    summary_path: pathlib.Path,
    report_path: pathlib.Path,
    promotion_ready_path: pathlib.Path,
    mode: str,
    message: str,
    current_hash: str,
) -> None:
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    effective_mode = mode if mode in ("warn", "enforce") else "warn"
    recommended_mode = "enforce" if mode == "enforce" else "warn"
    summary = [
        "## Determinism Stability Window",
        "",
        f"- Mode: `{mode}`",
        f"- Effective mode: `{effective_mode}`",
        f"- Current hash: `{current_hash or 'unavailable'}`",
        f"- Status: issue",
        f"- Promotion readiness: `unknown`",
        f"- Detail: {message}",
        "",
    ]
    summary_path.write_text("\n".join(summary), encoding="utf-8")

    payload = {
        "mode": mode,
        "status": "issue",
        "currentHash": current_hash,
        "effectiveMode": effective_mode,
        "message": message,
        "promotionReady": None,
        "recommendedMode": recommended_mode,
        "historicalSampleCount": 0,
        "uniqueHashes": [current_hash] if current_hash else [],
        "samples": [],
    }
    report_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    write_promotion_ready_flag(promotion_ready_path, None)


def request_json(url: str, token: str) -> dict:
    req = urllib.request.Request(url)
    req.add_header("Accept", "application/vnd.github+json")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    with urllib.request.urlopen(req, timeout=30) as response:
        return json.load(response)


def request_bytes(url: str, token: str) -> bytes:
    req = urllib.request.Request(url)
    req.add_header("Accept", "application/vnd.github+json")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    with urllib.request.urlopen(req, timeout=30) as response:
        return response.read()


def read_hash_file(path: pathlib.Path) -> str:
    value = path.read_text(encoding="utf-8").strip()
    if not HASH_RE.match(value):
        raise ValueError(f"invalid state hash in {path}: '{value}'")
    return value


def extract_current_hash(hash_root: pathlib.Path) -> str:
    files = sorted(hash_root.rglob("state_hash.txt"))
    if not files:
        raise FileNotFoundError(f"no state_hash.txt found under '{hash_root}'")

    hashes = [read_hash_file(p) for p in files]
    unique = sorted(set(hashes))
    if len(unique) != 1:
        raise ValueError(f"current run hash mismatch across artifacts: {unique}")
    return unique[0]


def iter_successful_runs(repo: str, branch: str, token: str) -> Iterable[dict]:
    page = 1
    while True:
        query = urllib.parse.urlencode(
            {
                "branch": branch,
                "status": "completed",
                "event": "push",
                "per_page": 100,
                "page": page,
            }
        )
        url = f"https://api.github.com/repos/{repo}/actions/runs?{query}"
        payload = request_json(url, token)
        runs = payload.get("workflow_runs", [])
        if not runs:
            break
        for run in runs:
            if run.get("conclusion") == "success":
                yield run
        page += 1


def read_run_hash(repo: str, run_id: int, token: str) -> str | None:
    url = f"https://api.github.com/repos/{repo}/actions/runs/{run_id}/artifacts?per_page=100"
    payload = request_json(url, token)
    artifacts = payload.get("artifacts", [])
    hash_values: list[str] = []

    for artifact in artifacts:
        name = str(artifact.get("name", ""))
        if not name.startswith(ARTIFACT_PREFIX):
            continue
        if artifact.get("expired", False):
            continue
        archive_url = artifact.get("archive_download_url")
        if not archive_url:
            continue
        raw = request_bytes(archive_url, token)
        with zipfile.ZipFile(io.BytesIO(raw)) as zf:
            members = [n for n in zf.namelist() if n.endswith("state_hash.txt")]
            if not members:
                continue
            with zf.open(members[0]) as fp:
                value = fp.read().decode("utf-8").strip()
                if HASH_RE.match(value):
                    hash_values.append(value)

    if not hash_values:
        return None

    unique = sorted(set(hash_values))
    if len(unique) != 1:
        return "__INTERNAL_MISMATCH__"
    return unique[0]


def write_summary(
    summary_path: pathlib.Path,
    current_hash: str,
    historical: list[RunHash],
    unique_hashes: list[str],
    min_samples: int,
    promotion_min_samples: int,
    promotion_ready: bool,
    recommended_mode: str,
    mode: str,
    effective_mode: str,
) -> None:
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    lines.append("## Determinism Stability Window")
    lines.append("")
    lines.append(f"- Mode: `{mode}`")
    lines.append(f"- Effective mode: `{effective_mode}`")
    lines.append(f"- Current hash: `{current_hash}`")
    lines.append(f"- Historical samples considered: `{len(historical)}`")
    lines.append(f"- Required minimum samples: `{min_samples}`")
    lines.append(f"- Promotion minimum samples: `{promotion_min_samples}`")
    lines.append(f"- Unique hashes in window+current: `{len(unique_hashes)}`")
    lines.append(f"- Promotion readiness: `{'ready' if promotion_ready else 'not-ready'}`")
    lines.append(f"- Recommended mode: `{recommended_mode}`")
    lines.append("")
    if unique_hashes:
        lines.append("### Hashes")
        for value in unique_hashes:
            lines.append(f"- `{value}`")
        lines.append("")
    lines.append("### Historical Samples")
    if historical:
        for sample in historical:
            lines.append(
                f"- run #{sample.run_number} (attempt {sample.run_attempt}, id {sample.run_id}): "
                f"`{sample.hash_value}` - {sample.html_url}"
            )
    else:
        lines.append("- no historical samples with determinism hash artifact found")
    lines.append("")
    summary_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_json_report(
    report_path: pathlib.Path,
    current_hash: str,
    historical: list[RunHash],
    unique_hashes: list[str],
    promotion_min_samples: int,
    promotion_ready: bool,
    recommended_mode: str,
    mode: str,
    effective_mode: str,
) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "mode": mode,
        "effectiveMode": effective_mode,
        "currentHash": current_hash,
        "historicalSampleCount": len(historical),
        "uniqueHashes": unique_hashes,
        "promotionMinSamples": promotion_min_samples,
        "promotionReady": promotion_ready,
        "recommendedMode": recommended_mode,
        "samples": [
            {
                "runId": sample.run_id,
                "runNumber": sample.run_number,
                "runAttempt": sample.run_attempt,
                "hash": sample.hash_value,
                "url": sample.html_url,
            }
            for sample in historical
        ],
    }
    report_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    hash_root = pathlib.Path(args.hash_root)

    current_hash = ""
    try:
        current_hash = extract_current_hash(hash_root)
    except (FileNotFoundError, ValueError) as exc:
        message = str(exc)
        write_issue_outputs(
            pathlib.Path(args.summary_path),
            pathlib.Path(args.json_path),
            pathlib.Path(args.promotion_ready_path),
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)

    if not args.repo:
        message = "missing --repo (or GITHUB_REPOSITORY)"
        write_issue_outputs(
            pathlib.Path(args.summary_path),
            pathlib.Path(args.json_path),
            pathlib.Path(args.promotion_ready_path),
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)

    token = os.environ.get("GITHUB_TOKEN", "")
    if not token:
        message = "missing GITHUB_TOKEN; cannot evaluate historical determinism stability window"
        write_issue_outputs(
            pathlib.Path(args.summary_path),
            pathlib.Path(args.json_path),
            pathlib.Path(args.promotion_ready_path),
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)

    historical: list[RunHash] = []

    try:
        for run in iter_successful_runs(args.repo, args.branch, token):
            if run.get("name") != args.workflow_name:
                continue
            run_id = int(run.get("id", 0))
            if run_id == args.current_run_id:
                continue
            value = read_run_hash(args.repo, run_id, token)
            if value is None:
                continue
            if value == "__INTERNAL_MISMATCH__":
                message = f"historical run {run_id} has internal cross-runner hash mismatch"
                write_issue_outputs(
                    pathlib.Path(args.summary_path),
                    pathlib.Path(args.json_path),
                    pathlib.Path(args.promotion_ready_path),
                    args.mode,
                    message,
                    current_hash,
                )
                return fail_or_warn(args.mode, message)
            historical.append(
                RunHash(
                    run_id=run_id,
                    run_number=int(run.get("run_number", 0)),
                    run_attempt=int(run.get("run_attempt", 1)),
                    html_url=str(run.get("html_url", "")),
                    hash_value=value,
                )
            )
            if len(historical) >= args.window:
                break
    except urllib.error.URLError as exc:
        message = f"failed to query GitHub Actions API: {exc}"
        write_issue_outputs(
            pathlib.Path(args.summary_path),
            pathlib.Path(args.json_path),
            pathlib.Path(args.promotion_ready_path),
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)
    except Exception as exc:  # noqa: BLE001
        message = f"unexpected stability window error: {exc}"
        write_issue_outputs(
            pathlib.Path(args.summary_path),
            pathlib.Path(args.json_path),
            pathlib.Path(args.promotion_ready_path),
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)

    all_hashes = [current_hash] + [item.hash_value for item in historical]
    unique_hashes = sorted(set(all_hashes))
    promotion_ready = len(historical) >= args.promotion_min_samples and len(unique_hashes) == 1
    effective_mode = args.mode
    if args.mode == "adaptive":
        effective_mode = "enforce" if promotion_ready else "warn"
    recommended_mode = "enforce" if promotion_ready else "warn"

    write_summary(
        pathlib.Path(args.summary_path),
        current_hash=current_hash,
        historical=historical,
        unique_hashes=unique_hashes,
        min_samples=args.min_samples,
        promotion_min_samples=args.promotion_min_samples,
        promotion_ready=promotion_ready,
        recommended_mode=recommended_mode,
        mode=args.mode,
        effective_mode=effective_mode,
    )
    write_json_report(
        pathlib.Path(args.json_path),
        current_hash=current_hash,
        historical=historical,
        unique_hashes=unique_hashes,
        promotion_min_samples=args.promotion_min_samples,
        promotion_ready=promotion_ready,
        recommended_mode=recommended_mode,
        mode=args.mode,
        effective_mode=effective_mode,
    )
    write_promotion_ready_flag(pathlib.Path(args.promotion_ready_path), promotion_ready)

    print(
        "[determinism-stability] samples="
        f"{len(historical)} unique_hashes={len(unique_hashes)} mode={args.mode} "
        f"effective_mode={effective_mode} promotion_ready={promotion_ready}"
    )

    if len(historical) < args.min_samples:
        return fail_or_warn(
            effective_mode,
            f"insufficient historical samples for determinism stability window "
            f"({len(historical)} < {args.min_samples})",
        )

    if len(unique_hashes) > 1:
        return fail_or_warn(
            effective_mode,
            "determinism stability window detected multiple distinct hashes",
        )

    if args.mode != "enforce" and promotion_ready:
        print(
            "::notice::determinism stability window meets promotion criteria; "
            "recommended mode is enforce"
        )

    print("[determinism-stability] stability window is consistent.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
