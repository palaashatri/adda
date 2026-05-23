"""Speech-to-text engine: faster-whisper if available, else transformers Whisper."""
from __future__ import annotations

from typing import Optional

from core.device import DEVICE, get_torch_dtype
from core.logger import logger
from core.scheduler import CancelledError, EngineRequest, EngineResponse
from models.registry import registry


class SpeechEngine:
    """Dual-backend ASR wrapper."""

    def __init__(self) -> None:
        self.backend: Optional[str] = None
        self.handle = None
        self.current_model_path: Optional[str] = None

    def _load(self, model_path: str) -> None:
        if self.current_model_path == model_path and self.handle is not None:
            return

        try:
            from faster_whisper import WhisperModel
            compute_type = "float16" if DEVICE == "cuda" else "int8"
            device = "cuda" if DEVICE == "cuda" else "cpu"
            logger.info(f"Loading ASR via faster-whisper ({device}/{compute_type})")
            self.handle = WhisperModel(model_path, device=device, compute_type=compute_type)
            self.backend = "faster_whisper"
            self.current_model_path = model_path
            return
        except Exception as exc:
            logger.warning(f"faster-whisper unavailable ({exc}); using transformers.")

        from transformers import pipeline
        import torch  # noqa: F401 — transformers reads torch via its own import
        logger.info(f"Loading ASR via transformers from {model_path}")
        self.handle = pipeline(
            "automatic-speech-recognition",
            model=model_path,
            device=0 if DEVICE == "cuda" else -1,
            torch_dtype=get_torch_dtype(),
        )
        self.backend = "transformers"
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        model_info = registry.get_active_model("speech")
        if not model_info:
            return EngineResponse(False, error="No active speech model selected.")
        try:
            self._load(model_info.local_path)
            token = request.cancel_token
            if token:
                token.raise_if_cancelled()

            audio_path = request.prompt
            if self.backend == "faster_whisper":
                segments, _info = self.handle.transcribe(audio_path)
                text = " ".join(seg.text.strip() for seg in segments)
            else:
                result = self.handle(audio_path)
                text = result["text"]

            return EngineResponse(
                True,
                text,
                metadata={
                    "audio": audio_path,
                    "model": model_info.name,
                    "backend": self.backend,
                },
            )
        except CancelledError:
            return EngineResponse(False, error="Cancelled.")
        except Exception as exc:
            logger.error(f"Speech transcription failed: {exc}")
            return EngineResponse(False, error=str(exc))


engine_speech = SpeechEngine()
