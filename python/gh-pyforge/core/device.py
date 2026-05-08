from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class DeviceInfo:
    """Information about the local compute device."""

    name: str
    has_cuda: bool
    backend: str


def detect_device() -> DeviceInfo:
    """Detect the best available inference device."""

    try:
        import torch

        has_cuda = torch.cuda.is_available()
        backend = "cuda" if has_cuda else "cpu"
        return DeviceInfo(name=backend, has_cuda=has_cuda, backend=backend)
    except Exception:
        return DeviceInfo(name="cpu", has_cuda=False, backend="cpu")
