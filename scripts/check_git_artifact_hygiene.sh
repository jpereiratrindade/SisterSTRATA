#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v git >/dev/null 2>&1; then
    echo "[git-hygiene-guard] git is required."
    exit 2
fi

FAILED=0

check_tracked_pathspec() {
    local label="$1"
    shift
    local pathspecs=("$@")
    local matches
    matches="$(git ls-files -- "${pathspecs[@]}")"

    if [[ -n "${matches}" ]]; then
        echo "[git-hygiene-guard][FAIL] ${label}"
        printf '%s\n' "${matches}"
        FAILED=1
    else
        echo "[git-hygiene-guard][OK] ${label}"
    fi
}

check_tracked_pathspec \
    "no tracked object artifacts (*.o, *.obj, *.lo) in repository index" \
    "*.o" "*.obj" "*.lo"

check_tracked_pathspec \
    "no tracked build output directories in repository index" \
    "build/**" "build-*/**"

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[git-hygiene-guard] repository artifact hygiene checks failed."
    exit 1
fi

echo "[git-hygiene-guard] repository artifact hygiene checks passed."
