"""Text-to-speech engine. Delegates to the audio (Bark) engine for now.

Kept as a separate symbol so the Speech tab can offer ASR and TTS
side-by-side without conflating the two pipelines.
"""
from __future__ import annotations

from engines.engine_audio import engine_audio
from core.scheduler import EngineRequest, EngineResponse


class TTSEngine:
    """Thin wrapper that re-uses the active audio model for TTS."""

    def run(self, request: EngineRequest) -> EngineResponse:
        return engine_audio.run(request)


engine_tts = TTSEngine()
