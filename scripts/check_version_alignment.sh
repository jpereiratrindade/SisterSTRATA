#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

if ! command -v rg >/dev/null 2>&1; then
    echo "[version-alignment-guard] ripgrep (rg) is required."
    exit 2
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "[version-alignment-guard] python3 is required."
    exit 2
fi

FAILED=0

extract_semver_from_line() {
    local line="$1"
    sed -E 's/.*([0-9]+\.[0-9]+\.[0-9]+).*/\1/' <<<"${line}"
}

read_cmake_version() {
    local line
    line="$(rg -n '^project\(SisterSTRATA VERSION [0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt | head -n 1 | cut -d: -f2- || true)"
    [[ -n "${line}" ]] || return 1
    extract_semver_from_line "${line}"
}

read_doxy_version() {
    local line
    line="$(rg -n '^PROJECT_NUMBER\s*=\s*"[0-9]+\.[0-9]+\.[0-9]+"' Doxyfile | head -n 1 | cut -d: -f2- || true)"
    [[ -n "${line}" ]] || return 1
    extract_semver_from_line "${line}"
}

read_engine_json_version() {
    python3 - <<'PY'
import json
from pathlib import Path

data = json.loads(Path("docs/SCIENTIFIC_MODEL_VERSION.json").read_text(encoding="utf-8"))
print(str(data.get("engineVersion", "")).strip())
PY
}

read_readme_version() {
    local line
    line="$(rg -n '^## Principais Funcionalidades \(v[0-9]+\.[0-9]+\.[0-9]+\)' README.md | head -n 1 | cut -d: -f2- || true)"
    [[ -n "${line}" ]] || return 1
    extract_semver_from_line "${line}"
}

read_changelog_top_version() {
    local line
    line="$(rg -n '^## \[v[0-9]+\.[0-9]+\.[0-9]+\]' CHANGELOG.md | head -n 1 | cut -d: -f2- || true)"
    [[ -n "${line}" ]] || return 1
    extract_semver_from_line "${line}"
}

check_equals() {
    local label="$1"
    local actual="$2"
    local expected="$3"
    if [[ "${actual}" == "${expected}" ]]; then
        echo "[version-alignment-guard][OK] ${label}: ${actual}"
    else
        echo "[version-alignment-guard][FAIL] ${label}: got '${actual}', expected '${expected}'"
        FAILED=1
    fi
}

main_version="$(read_cmake_version || true)"
if [[ -z "${main_version}" ]]; then
    echo "[version-alignment-guard][FAIL] could not read version from CMakeLists.txt"
    exit 1
fi
echo "[version-alignment-guard] baseline engine version from CMakeLists.txt: ${main_version}"

doxy_version="$(read_doxy_version || true)"
json_version="$(read_engine_json_version || true)"
readme_version="$(read_readme_version || true)"
changelog_version="$(read_changelog_top_version || true)"

check_equals "Doxyfile PROJECT_NUMBER" "${doxy_version}" "${main_version}"
check_equals "SCIENTIFIC_MODEL_VERSION.json engineVersion" "${json_version}" "${main_version}"
check_equals "README feature header version" "${readme_version}" "${main_version}"
check_equals "CHANGELOG top release version" "${changelog_version}" "${main_version}"

release_note_path="docs/releases/v${main_version}.md"
if [[ -f "${release_note_path}" ]]; then
    echo "[version-alignment-guard][OK] release notes exist: ${release_note_path}"
else
    echo "[version-alignment-guard][FAIL] missing release notes: ${release_note_path}"
    FAILED=1
fi

if rg -q "releases/v${main_version}\.md" docs/README.md; then
    echo "[version-alignment-guard][OK] docs/README index contains v${main_version}"
else
    echo "[version-alignment-guard][FAIL] docs/README index missing releases/v${main_version}.md"
    FAILED=1
fi

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[version-alignment-guard] version alignment checks failed."
    exit 1
fi

echo "[version-alignment-guard] version alignment checks passed."
