from __future__ import annotations

import time
from typing import Any

from . import EngineRequest, EngineResponse
from core.task_queue import CancellationToken


def run(request: EngineRequest, cancel_token: CancellationToken | None = None) -> EngineResponse:
    """Run an audio workflow in the background."""

    try:
        for progress in range(4):
            if cancel_token and cancel_token.is_cancelled:
                return EngineResponse(success=False, output=None, metadata={}, error="Cancelled")
            time.sleep(0.1)
        output = {
            "audio_path": "~/.pyforge/history/latest-audio.wav",
            "prompt": request.prompt,
        }
        metadata: dict[str, Any] = {
            "model": request.model,
            "voice": request.parameters.get("voice", "default"),
        }
        return EngineResponse(success=True, output=output, metadata=metadata, error=None)
    except Exception as error:
        return EngineResponse(success=False, output=None, metadata={}, error=str(error))
