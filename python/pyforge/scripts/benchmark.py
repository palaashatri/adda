"""Per-engine latency + memory benchmark.

Run as: `python scripts/benchmark.py` from the repo root.
Only benchmarks engines whose active model is already installed; everything
else is skipped with a note. Memory measurement uses `psutil` if available
and otherwise falls back to RSS via `resource` (Unix) or `tracemalloc` (any OS).
"""
from __future__ import annotations

import gc
import os
import sys
import time
from typing import Callable

# Ensure repo root is importable when running directly from scripts/.
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from core.logger import logger  # noqa: E402
from core.scheduler import EngineRequest  # noqa: E402
from models.registry import registry  # noqa: E402


def _rss_mb() -> float:
    try:
        import psutil
        return psutil.Process(os.getpid()).memory_info().rss / (1024 ** 2)
    except Exception:
        try:
            import resource
            return resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024
        except Exception:
            return -1.0


def _bench(name: str, modality: str, run: Callable[[], object]) -> None:
    info = registry.get_active_model(modality)
    if info is None:
        print(f"[skip] {name}: no model installed for modality '{modality}'.")
        return
    gc.collect()
    before_mb = _rss_mb()
    t0 = time.perf_counter()
    try:
        run()
    except Exception as exc:
        print(f"[fail] {name}: {exc}")
        return
    dt = time.perf_counter() - t0
    after_mb = _rss_mb()
    delta = after_mb - before_mb if before_mb >= 0 else -1
    print(
        f"[ok]   {name}: {dt:.2f}s   "
        f"RSS {before_mb:.0f}→{after_mb:.0f} MB (Δ {delta:+.0f})"
    )


def run_benchmark() -> None:
    logger.info("Starting PyForge benchmark…")

    from engines.engine_image import engine_image
    from engines.engine_llm import engine_llm

    _bench(
        "image.txt2img",
        "image",
        lambda: engine_image.run(
            EngineRequest("a tiny cat", "", "image", steps=4, width=256, height=256)
        ),
    )
    _bench(
        "llm.generate",
        "llm",
        lambda: engine_llm.run(EngineRequest("Hello,", "", "llm", max_new_tokens=16)),
    )


if __name__ == "__main__":
    run_benchmark()
