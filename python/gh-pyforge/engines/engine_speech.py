from __future__ import annotations

import time
from typing import Any

from . import EngineRequest, EngineResponse
from core.task_queue import CancellationToken


def run(request: EngineRequest, cancel_token: CancellationToken | None = None) -> EngineResponse:
    """Run a speech workflow in the background."""

    try:
        for progress in range(3):
            if cancel_token and cancel_token.is_cancelled:
                return EngineResponse(success=False, output=None, metadata={}, error="Cancelled")
            time.sleep(0.1)
        output = {
            "speech_path": "~/.pyforge/history/latest-speech.wav",
            "prompt": request.prompt,
        }
        metadata: dict[str, Any] = {
            "model": request.model,
            "format": request.parameters.get("format", "wav"),
        }
        return EngineResponse(success=True, output=output, metadata=metadata, error=None)
    except Exception as error:
        return EngineResponse(success=False, output=None, metadata={}, error=str(error))
