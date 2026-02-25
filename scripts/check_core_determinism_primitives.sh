#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v rg >/dev/null 2>&1; then
    echo "[core-determinism-guard] ripgrep (rg) is required."
    exit 2
fi

FAILED=0
MATCH_FILE="$(mktemp /tmp/core_determinism_guard_match.XXXXXX)"
cleanup() {
    rm -f "${MATCH_FILE}"
}
trap cleanup EXIT

run_guard() {
    local label="$1"
    local pattern="$2"

    if rg -n --glob '*.{hpp,cpp}' "${pattern}" src/core/domain >"${MATCH_FILE}"; then
        echo "[core-determinism-guard][FAIL] ${label}"
        cat "${MATCH_FILE}"
        FAILED=1
    else
        echo "[core-determinism-guard][OK] ${label}"
    fi
}

run_guard \
    "core/domain must not read wall-clock time at runtime" \
    'std::chrono::(system_clock|high_resolution_clock|steady_clock)::now\s*\(|std::time\s*\(\s*(nullptr|0)\s*\)|[^:]time\s*\(\s*(nullptr|0)\s*\)'

run_guard \
    "core/domain must not use nondeterministic random primitives" \
    'std::random_device|\brand\s*\(|\bsrand\s*\('

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[core-determinism-guard] deterministic primitive checks failed."
    exit 1
fi

echo "[core-determinism-guard] deterministic primitive checks passed."
