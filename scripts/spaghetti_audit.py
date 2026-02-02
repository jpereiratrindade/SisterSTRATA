#!/usr/bin/env python3
"""Quick include-based coupling audit for SisterSTRATA.

Usage:
  python3 scripts/spaghetti_audit.py [src_root]
"""
from __future__ import annotations

import collections
import pathlib
import re
import sys

INCLUDE_RE = re.compile(r'^#include\s+"([^"]+)"')


def layer(path: str) -> str:
    p = path.replace('\\', '/')
    if p.startswith('core/'): return 'core'
    if p.startswith('application/'): return 'application'
    if p.startswith('ui/'): return 'ui'
    if p.startswith('world3d/'): return 'world3d'
    if p.startswith('infrastructure/'): return 'infrastructure'
    if p.startswith('observational/') or p.startswith('src/observational/'): return 'observational'
    return 'other'


def audit(root: pathlib.Path) -> tuple[collections.Counter, list[tuple[int, pathlib.Path, collections.Counter]]]:
    edges: collections.Counter[tuple[str, str]] = collections.Counter()
    per_file: list[tuple[int, pathlib.Path, collections.Counter]] = []

    for file in root.rglob('*'):
        if file.suffix not in {'.cpp', '.hpp', '.h'}:
            continue
        try:
            text = file.read_text(encoding='utf-8', errors='ignore')
        except Exception:
            continue
        src_layer = layer(str(file.relative_to(root)))
        cross = 0
        by = collections.Counter()
        for line in text.splitlines():
            m = INCLUDE_RE.match(line.strip())
            if not m:
                continue
            tgt = layer(m.group(1))
            if src_layer != 'other' and tgt != 'other' and src_layer != tgt:
                edges[(src_layer, tgt)] += 1
                cross += 1
                by[tgt] += 1
        if cross:
            per_file.append((cross, file, by))

    per_file.sort(key=lambda x: (-x[0], str(x[1])))
    return edges, per_file


def main() -> int:
    root = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path('src')
    if not root.exists():
        print(f'ERROR: src root not found: {root}')
        return 1

    edges, per_file = audit(root)

    print('Cross-layer include counts:')
    for (a, b), c in sorted(edges.items(), key=lambda x: (-x[1], x[0])):
        print(f'{a} -> {b}: {c}')

    print('\nTop cross-include hotspots:')
    for cross, file, by in per_file[:20]:
        rel = file.relative_to(root)
        detail = ', '.join(f'{k}:{v}' for k, v in by.most_common())
        print(f'{cross:2d} {rel} [{detail}]')

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
