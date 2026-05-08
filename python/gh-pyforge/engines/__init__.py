from __future__ import annotations

from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class EngineRequest:
    """A request object passed to an engine."""

    prompt: str
    model: str
    modality: str
    parameters: dict[str, Any]


@dataclass(frozen=True)
class EngineResponse:
    """A standardized engine response."""

    success: bool
    output: Any
    metadata: dict[str, Any]
    error: str | None = None
