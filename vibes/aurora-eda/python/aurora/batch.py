"""I7 — Batch / headless mode entry point.

Usage:
    python -m aurora.batch <script.py> [--key=value ...]

The given script is executed with `args` available as a dict in its globals.
"""

import argparse
import runpy
import sys
from typing import Dict


def _parse_kv(argv) -> Dict[str, str]:
    out = {}
    for a in argv:
        if a.startswith("--") and "=" in a:
            k, v = a[2:].split("=", 1)
            out[k] = v
    return out


def main(argv=None):
    argv = list(argv if argv is not None else sys.argv[1:])
    if not argv:
        print("Usage: python -m aurora.batch <script.py> [--key=value ...]")
        return 2
    script = argv[0]
    args = _parse_kv(argv[1:])
    runpy.run_path(script, init_globals={"args": args, "__name__": "__aurora_batch__"})
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
