#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v rg >/dev/null 2>&1; then
    echo "[core-boundary-guard] ripgrep (rg) is required."
    exit 2
fi

FAILED=0

run_guard() {
    local label="$1"
    local search_dir="$2"
    local pattern="$3"

    if rg -n --glob '*.{hpp,cpp}' "${pattern}" "${search_dir}" >/tmp/core_domain_guard_match.txt; then
        echo "[core-boundary-guard][FAIL] ${label}"
        cat /tmp/core_domain_guard_match.txt
        FAILED=1
    else
        echo "[core-boundary-guard][OK] ${label}"
    fi
}

# Core domain must not depend on outer layers.
run_guard \
    "core/domain must not include outer-layer headers (application/ui/world3d/observational/infrastructure adapters)" \
    "src/core/domain" \
    '#include "(src/)?(application|ui|world3d|observational|infrastructure)/'

rm -f /tmp/core_domain_guard_match.txt

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[core-boundary-guard] core domain dependency checks failed."
    exit 1
fi

echo "[core-boundary-guard] all core domain dependency checks passed."
