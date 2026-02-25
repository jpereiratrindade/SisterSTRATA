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
    parser.add_argument("--mode", choices=("warn", "enforce"), default="warn")
    parser.add_argument("--summary-path", default="build/headless/determinism/stability_window_summary.md")
    parser.add_argument("--json-path", default="build/headless/determinism/stability_window_report.json")
    return parser.parse_args()


def fail_or_warn(mode: str, message: str) -> int:
    if mode == "enforce":
        print(f"[determinism-stability][FAIL] {message}")
        return 1
    print(f"::warning::{message}")
    return 0


def write_issue_outputs(
    summary_path: pathlib.Path,
    report_path: pathlib.Path,
    mode: str,
    message: str,
    current_hash: str,
) -> None:
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.parent.mkdir(parents=True, exist_ok=True)

    summary = [
        "## Determinism Stability Window",
        "",
        f"- Mode: `{mode}`",
        f"- Current hash: `{current_hash or 'unavailable'}`",
        f"- Status: issue",
        f"- Detail: {message}",
        "",
    ]
    summary_path.write_text("\n".join(summary), encoding="utf-8")

    payload = {
        "mode": mode,
        "status": "issue",
        "currentHash": current_hash,
        "message": message,
        "historicalSampleCount": 0,
        "uniqueHashes": [current_hash] if current_hash else [],
        "samples": [],
    }
    report_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


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
    mode: str,
) -> None:
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    lines.append("## Determinism Stability Window")
    lines.append("")
    lines.append(f"- Mode: `{mode}`")
    lines.append(f"- Current hash: `{current_hash}`")
    lines.append(f"- Historical samples considered: `{len(historical)}`")
    lines.append(f"- Required minimum samples: `{min_samples}`")
    lines.append(f"- Unique hashes in window+current: `{len(unique_hashes)}`")
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
    mode: str,
) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "mode": mode,
        "currentHash": current_hash,
        "historicalSampleCount": len(historical),
        "uniqueHashes": unique_hashes,
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
            args.mode,
            message,
            current_hash,
        )
        return fail_or_warn(args.mode, message)

    all_hashes = [current_hash] + [item.hash_value for item in historical]
    unique_hashes = sorted(set(all_hashes))

    write_summary(
        pathlib.Path(args.summary_path),
        current_hash=current_hash,
        historical=historical,
        unique_hashes=unique_hashes,
        min_samples=args.min_samples,
        mode=args.mode,
    )
    write_json_report(
        pathlib.Path(args.json_path),
        current_hash=current_hash,
        historical=historical,
        unique_hashes=unique_hashes,
        mode=args.mode,
    )

    print(
        "[determinism-stability] samples="
        f"{len(historical)} unique_hashes={len(unique_hashes)} mode={args.mode}"
    )

    if len(historical) < args.min_samples:
        return fail_or_warn(
            args.mode,
            f"insufficient historical samples for determinism stability window "
            f"({len(historical)} < {args.min_samples})",
        )

    if len(unique_hashes) > 1:
        return fail_or_warn(
            args.mode,
            "determinism stability window detected multiple distinct hashes",
        )

    print("[determinism-stability] stability window is consistent.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
