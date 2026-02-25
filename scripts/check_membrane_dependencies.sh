#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v rg >/dev/null 2>&1; then
    echo "[membrane-guard] ripgrep (rg) is required."
    exit 2
fi

FAILED=0
MATCH_FILE="$(mktemp /tmp/membrane_guard_match.XXXXXX)"
cleanup() {
    rm -f "${MATCH_FILE}"
}
trap cleanup EXIT

run_guard() {
    local label="$1"
    local search_dir="$2"
    local pattern="$3"

    if rg -n --glob '*.{hpp,cpp}' "${pattern}" "${search_dir}" >"${MATCH_FILE}"; then
        echo "[membrane-guard][FAIL] ${label}"
        cat "${MATCH_FILE}"
        FAILED=1
    else
        echo "[membrane-guard][OK] ${label}"
    fi
}

# Infrastructure domain must not depend on ecological/fourth-dimension modules.
run_guard \
    "infrastructure must not include ecological/fourth-dimension domain headers" \
    "src/core/domain/infrastructure" \
    '#include ".*core/domain/(vegetation|hydro|world|fourth_dimension|simulation)/'

# Infrastructure domain must not depend on outer layers.
run_guard \
    "infrastructure must not include outer-layer modules" \
    "src/core/domain/infrastructure" \
    '#include ".*(src/)?(application|ui|world3d|observational)/'

# Fourth Dimension domain must not depend directly on infrastructure domain.
run_guard \
    "fourth-dimension must not include infrastructure domain headers" \
    "src/core/domain/fourth_dimension" \
    '#include ".*core/domain/infrastructure/'

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[membrane-guard] dependency membrane checks failed."
    exit 1
fi

echo "[membrane-guard] all dependency membrane checks passed."
