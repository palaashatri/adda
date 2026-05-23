"""Standardized engine I/O dataclasses and the cancellation primitive."""
from __future__ import annotations

import threading
from dataclasses import dataclass, field
from typing import Any, Optional


class CancellationToken:
    """Thread-safe cooperative cancellation flag.

    Engines should periodically call `.raise_if_cancelled()` (or check `.is_set()`)
    inside long inference loops so cancellation actually preempts work, rather
    than relying on `Future.cancel()` which only works pre-start.
    """

    def __init__(self) -> None:
        self._event = threading.Event()

    def cancel(self) -> None:
        self._event.set()

    def is_set(self) -> bool:
        return self._event.is_set()

    def raise_if_cancelled(self) -> None:
        if self._event.is_set():
            raise CancelledError("Task cancelled by user.")


class CancelledError(RuntimeError):
    """Raised inside engines when their cancellation token fires."""


@dataclass
class EngineRequest:
    """Standardized request object for all engines."""
    prompt: str
    model_id: str
    modality: str
    kwargs: dict = field(default_factory=dict)
    cancel_token: Optional[CancellationToken] = None

    def __init__(
        self,
        prompt: str,
        model_id: str,
        modality: str,
        cancel_token: Optional[CancellationToken] = None,
        **kwargs: Any,
    ) -> None:
        self.prompt = prompt
        self.model_id = model_id
        self.modality = modality
        self.kwargs = kwargs
        self.cancel_token = cancel_token


@dataclass
class EngineResponse:
    """Standardized response object for all engines."""
    success: bool
    output: Any = None
    metadata: dict = field(default_factory=dict)
    error: Optional[str] = None

    def __init__(
        self,
        success: bool,
        output: Any = None,
        metadata: Optional[dict] = None,
        error: Optional[str] = None,
    ) -> None:
        self.success = success
        self.output = output
        self.metadata = metadata or {}
        self.error = error
