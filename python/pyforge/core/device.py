"""Device + dtype selection. CUDA > MPS > CPU, with VRAM-aware dtype."""
from __future__ import annotations

import os
from typing import Optional

import torch

from core.logger import logger


def get_device() -> str:
    """Detect and return the best available device string."""
    if torch.cuda.is_available():
        logger.info("CUDA detected.")
        return "cuda"
    if hasattr(torch.backends, "mps") and torch.backends.mps.is_available():
        logger.info("MPS (Apple Silicon) detected.")
        return "mps"
    logger.info("No GPU detected. Falling back to CPU.")
    return "cpu"


DEVICE: str = get_device()


def get_vram_gb() -> Optional[float]:
    """Return total CUDA VRAM in GB, or None if not on CUDA."""
    if DEVICE != "cuda":
        return None
    try:
        props = torch.cuda.get_device_properties(0)
        return props.total_memory / (1024 ** 3)
    except Exception as exc:
        logger.warning(f"Could not query VRAM: {exc}")
        return None


def get_torch_dtype() -> torch.dtype:
    """Pick a sensible torch dtype for the active device + VRAM budget.

    - CPU: float32 (no fp16 wins on CPU for these models)
    - MPS: float32 (fp16 ops are flaky on Apple Silicon for many ops)
    - CUDA <4GB: float32 (fp16 can OOM during conversion on tiny GPUs)
    - CUDA otherwise: float16
    """
    if DEVICE == "cpu":
        return torch.float32
    if DEVICE == "mps":
        return torch.float32
    vram = get_vram_gb()
    if vram is not None and vram < 4.0:
        return torch.float32
    return torch.float16


def should_compile() -> bool:
    """Whether torch.compile is worth attempting (Ampere+ on CUDA)."""
    if DEVICE != "cuda":
        return False
    if os.environ.get("PYFORGE_NO_COMPILE"):
        return False
    try:
        major, _ = torch.cuda.get_device_capability(0)
        return major >= 8
    except Exception:
        return False


def tune_cpu_threads() -> None:
    """Use most-but-not-all logical cores when running on CPU."""
    if DEVICE != "cpu":
        return
    try:
        cores = os.cpu_count() or 4
        threads = max(1, cores - 1)
        torch.set_num_threads(threads)
        logger.info(f"Set torch CPU threads = {threads}")
    except Exception as exc:
        logger.warning(f"Could not tune CPU threads: {exc}")


tune_cpu_threads()
