from __future__ import annotations

import time


def run_benchmark() -> None:
    """Run a simple performance benchmark for engine stubs."""

    start = time.perf_counter()
    for _ in range(10):
        time.sleep(0.03)
    elapsed = time.perf_counter() - start
    print(f"PyForge benchmark completed in {elapsed:.3f} seconds.")


if __name__ == "__main__":
    run_benchmark()
