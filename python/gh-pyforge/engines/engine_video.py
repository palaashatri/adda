from __future__ import annotations

import time
from typing import Any

from . import EngineRequest, EngineResponse
from core.task_queue import CancellationToken


def run(request: EngineRequest, cancel_token: CancellationToken | None = None) -> EngineResponse:
    """Run a video workflow in the background."""

    try:
        for progress in range(6):
            if cancel_token and cancel_token.is_cancelled:
                return EngineResponse(success=False, output=None, metadata={}, error="Cancelled")
            time.sleep(0.1)
        output = {
            "video_path": "~/.pyforge/history/latest-video.mp4",
            "prompt": request.prompt,
        }
        metadata: dict[str, Any] = {
            "model": request.model,
            "duration": request.parameters.get("duration", "5s"),
        }
        return EngineResponse(success=True, output=output, metadata=metadata, error=None)
    except Exception as error:
        return EngineResponse(success=False, output=None, metadata={}, error=str(error))
