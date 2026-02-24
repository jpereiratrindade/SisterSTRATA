#!/usr/bin/env bash
set -euo pipefail

HASH_ROOT="${1:-determinism-hashes}"
MODE="${2:-warn}"

if [[ "${MODE}" != "warn" && "${MODE}" != "enforce" ]]; then
    echo "[determinism-compare][FAIL] invalid mode '${MODE}' (expected: warn|enforce)."
    exit 2
fi

mapfile -t files < <(find "${HASH_ROOT}" -type f -name 'state_hash.txt' | sort)

if [[ "${#files[@]}" -lt 2 ]]; then
    message="[determinism-compare] not enough determinism hash artifacts to compare (${#files[@]} found)."
    if [[ "${MODE}" == "enforce" ]]; then
        echo "[determinism-compare][FAIL] ${message}"
        exit 1
    fi
    echo "::warning::${message}"
    exit 0
fi

base_hash="$(tr -d ' \n\r\t' < "${files[0]}")"
if [[ -z "${base_hash}" ]]; then
    echo "[determinism-compare][FAIL] empty hash in ${files[0]}"
    exit 1
fi

mismatch=0
for f in "${files[@]}"; do
    current="$(tr -d ' \n\r\t' < "${f}")"
    if [[ -z "${current}" ]]; then
        echo "[determinism-compare][FAIL] empty hash in ${f}"
        exit 1
    fi
    echo "[determinism-compare] ${f}: ${current}"
    if [[ "${current}" != "${base_hash}" ]]; then
        mismatch=1
    fi
done

if [[ "${mismatch}" -eq 0 ]]; then
    echo "[determinism-compare] cross-runner deterministic hashes are identical."
    exit 0
fi

message="cross-runner determinism hash mismatch detected."
if [[ "${MODE}" == "enforce" ]]; then
    echo "[determinism-compare][FAIL] ${message}"
    exit 1
fi

echo "::warning::${message}"
