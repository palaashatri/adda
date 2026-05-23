"""Cleanup script: remove orphaned model dirs, temp output files, and old history.

Run as: `python scripts/cleanup.py [--dry-run] [--keep-history-days N]`.
"""
from __future__ import annotations

import argparse
import os
import shutil
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from core.config import HISTORY_DIR, MODELS_DIR  # noqa: E402
from core.logger import logger  # noqa: E402


def _orphan_model_dirs() -> list[Path]:
    """Return model directories whose model.json metadata is missing."""
    orphans: list[Path] = []
    if not MODELS_DIR.exists():
        return orphans
    for modality_dir in MODELS_DIR.iterdir():
        if not modality_dir.is_dir():
            continue
        for model_dir in modality_dir.iterdir():
            if not model_dir.is_dir():
                continue
            if not (model_dir / "model.json").exists():
                orphans.append(model_dir)
    return orphans


def _temp_outputs() -> list[Path]:
    """Find pyforge_* files in the system temp dir."""
    tmp = Path(tempfile.gettempdir())
    return [p for p in tmp.glob("pyforge_*") if p.is_file()]


def _old_history(days: int) -> list[Path]:
    if not HISTORY_DIR.exists() or days <= 0:
        return []
    cutoff = time.time() - days * 86400
    return [p for p in HISTORY_DIR.glob("*.json") if p.stat().st_mtime < cutoff]


def cleanup(dry_run: bool, keep_history_days: int) -> None:
    orphans = _orphan_model_dirs()
    tmps = _temp_outputs()
    old_hist = _old_history(keep_history_days)
    total = len(orphans) + len(tmps) + len(old_hist)

    print(f"Orphan model dirs: {len(orphans)}")
    print(f"Temp output files: {len(tmps)}")
    print(f"History entries older than {keep_history_days}d: {len(old_hist)}")
    if total == 0:
        print("Nothing to clean.")
        return
    if dry_run:
        for p in orphans + tmps + old_hist:
            print(f"  would remove: {p}")
        return

    for d in orphans:
        try:
            shutil.rmtree(d)
            logger.info(f"removed orphan {d}")
        except Exception as exc:
            logger.error(f"failed to remove {d}: {exc}")
    for f in tmps + old_hist:
        try:
            f.unlink()
            logger.info(f"removed file {f}")
        except Exception as exc:
            logger.error(f"failed to remove {f}: {exc}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--keep-history-days", type=int, default=0,
                        help="Delete history entries older than N days (0 = never).")
    args = parser.parse_args()
    cleanup(args.dry_run, args.keep_history_days)


if __name__ == "__main__":
    main()
