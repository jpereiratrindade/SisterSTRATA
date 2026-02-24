#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v rg >/dev/null 2>&1; then
    echo "[exception-boundary-guard] ripgrep (rg) is required."
    exit 2
fi

MATCH_FILE="/tmp/exception_boundary_guard_match.txt"
FAILED=0

# Allowed broad catch(...) boundary shields.
ALLOWED_FILES=(
    "src/main.cpp"
    "src/world3d/Engine.cpp"
)

is_allowed_file() {
    local file="$1"
    for allowed in "${ALLOWED_FILES[@]}"; do
        if [[ "${file}" == "${allowed}" ]]; then
            return 0
        fi
    done
    return 1
}

if ! rg -n --glob '*.{hpp,cpp}' 'catch\s*\(\s*\.\.\.\s*\)' src >"${MATCH_FILE}"; then
    echo "[exception-boundary-guard][OK] no catch(...) usage found in src/"
    rm -f "${MATCH_FILE}"
    exit 0
fi

while IFS=: read -r file line_number _; do
    if is_allowed_file "${file}"; then
        echo "[exception-boundary-guard][OK] allowed boundary catch at ${file}:${line_number}"
    else
        echo "[exception-boundary-guard][FAIL] forbidden catch(...) at ${file}:${line_number}"
        FAILED=1
    fi
done < "${MATCH_FILE}"

rm -f "${MATCH_FILE}"

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[exception-boundary-guard] forbidden catch(...) usage detected."
    exit 1
fi

echo "[exception-boundary-guard] all catch(...) usage is boundary-allowlisted."
