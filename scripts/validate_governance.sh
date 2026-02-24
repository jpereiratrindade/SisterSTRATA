#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="${1:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
cd "${ROOT_DIR}"

FAILED=0

require_file() {
    local path="$1"
    if [[ ! -f "${path}" ]]; then
        echo "[governance-gate][FAIL] missing file: ${path}"
        FAILED=1
    else
        echo "[governance-gate][OK] file exists: ${path}"
    fi
}

require_doc_metadata() {
    local path="$1"
    local label="$2"

    if ! rg -q "^Owner:" "${path}"; then
        echo "[governance-gate][FAIL] ${label}: missing 'Owner:'"
        FAILED=1
    else
        echo "[governance-gate][OK] ${label}: owner declared"
    fi

    if ! rg -q "^Review cadence:" "${path}"; then
        echo "[governance-gate][FAIL] ${label}: missing 'Review cadence:'"
        FAILED=1
    else
        echo "[governance-gate][OK] ${label}: review cadence declared"
    fi
}

echo "[governance-gate] validating governance artifacts..."

require_file "docs/SCIENTIFIC_MODEL_VERSIONING.md"
require_file "docs/BREAKING_CHANGES_POLICY.md"
require_file "docs/RELEASE_STABILITY_CRITERIA.md"
require_file "docs/SCIENTIFIC_MODEL_VERSION.json"
require_file ".github/PULL_REQUEST_TEMPLATE.md"

if [[ -f "docs/SCIENTIFIC_MODEL_VERSIONING.md" ]]; then
    require_doc_metadata "docs/SCIENTIFIC_MODEL_VERSIONING.md" "SCIENTIFIC_MODEL_VERSIONING"
fi
if [[ -f "docs/BREAKING_CHANGES_POLICY.md" ]]; then
    require_doc_metadata "docs/BREAKING_CHANGES_POLICY.md" "BREAKING_CHANGES_POLICY"
fi
if [[ -f "docs/RELEASE_STABILITY_CRITERIA.md" ]]; then
    require_doc_metadata "docs/RELEASE_STABILITY_CRITERIA.md" "RELEASE_STABILITY_CRITERIA"
fi

if [[ -f "docs/SCIENTIFIC_MODEL_VERSION.json" ]]; then
    if ! python3 - <<'PY'
import json
import re
from pathlib import Path

path = Path("docs/SCIENTIFIC_MODEL_VERSION.json")
data = json.loads(path.read_text(encoding="utf-8"))

required = [
    "schemaVersion",
    "updatedAt",
    "owner",
    "engineVersion",
    "scientificModelVersion",
    "componentVersions",
]
missing = [k for k in required if k not in data]
if missing:
    raise SystemExit(f"[governance-gate][FAIL] missing keys in SCIENTIFIC_MODEL_VERSION.json: {missing}")

semver = re.compile(r"^\d+\.\d+\.\d+$")
for key in ("engineVersion", "scientificModelVersion"):
    value = str(data[key])
    if not semver.match(value):
        raise SystemExit(f"[governance-gate][FAIL] {key} must be semver MAJOR.MINOR.PATCH: got '{value}'")

if not isinstance(data["componentVersions"], dict) or not data["componentVersions"]:
    raise SystemExit("[governance-gate][FAIL] componentVersions must be a non-empty object")

for name, version in data["componentVersions"].items():
    if not semver.match(str(version)):
        raise SystemExit(f"[governance-gate][FAIL] component '{name}' must use semver: got '{version}'")

print("[governance-gate][OK] scientific model version JSON is valid")
PY
    then
        FAILED=1
    fi
fi

EVENT_NAME="${GITHUB_EVENT_NAME:-}"
EVENT_PATH="${GITHUB_EVENT_PATH:-}"

if [[ "${EVENT_NAME}" == "pull_request" || "${EVENT_NAME}" == "pull_request_target" ]]; then
    echo "[governance-gate] validating PR checklist..."
    if [[ -z "${EVENT_PATH}" || ! -f "${EVENT_PATH}" ]]; then
        echo "[governance-gate][FAIL] GITHUB_EVENT_PATH not available for PR validation"
        FAILED=1
    else
        if ! python3 - "${EVENT_PATH}" <<'PY'
import json
import re
import sys

event_path = sys.argv[1]
event = json.load(open(event_path, "r", encoding="utf-8"))
body = (event.get("pull_request") or {}).get("body") or ""

required_items = [
    "Versioning (engine + scientific) preenchido",
    "Nenhum breaking change sem documentacao explicita",
    "Todos testes deterministicos passando",
    "Resultados deterministicos validados",
    "Criterios de estabilidade atendidos",
    "Gate CI: validacao de versao cientifica ok",
]

if not body.strip():
    raise SystemExit("[governance-gate][FAIL] pull request body is empty; governance checklist is required")

missing = []
for item in required_items:
    pattern = re.compile(rf"^\s*-\s*\[[xX]\]\s*{re.escape(item)}\s*$", re.MULTILINE)
    if not pattern.search(body):
        missing.append(item)

if missing:
    raise SystemExit(
        "[governance-gate][FAIL] missing checked governance checklist items:\n- "
        + "\n- ".join(missing)
    )

print("[governance-gate][OK] PR governance checklist items are checked")
PY
        then
            FAILED=1
        fi
    fi
else
    echo "[governance-gate] PR checklist validation skipped for event: ${EVENT_NAME:-unknown}"
fi

if [[ "${FAILED}" -ne 0 ]]; then
    echo "[governance-gate] governance validation failed."
    exit 1
fi

echo "[governance-gate] governance validation passed."

