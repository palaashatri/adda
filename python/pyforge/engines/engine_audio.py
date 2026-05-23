"""Audio (TTS) engine: Bark via transformers."""
from __future__ import annotations

import os
import tempfile
from typing import Optional

import numpy as np
import scipy.io.wavfile as wavfile
import torch
from transformers import AutoProcessor, BarkModel

from core.device import DEVICE, get_torch_dtype
from core.logger import logger
from core.scheduler import CancelledError, EngineRequest, EngineResponse
from models.registry import registry


class AudioEngine:
    """Bark TTS wrapper."""

    def __init__(self) -> None:
        self.processor = None
        self.model = None
        self.current_model_path: Optional[str] = None

    def _load(self, model_path: str) -> None:
        if self.current_model_path == model_path and self.model is not None:
            return
        logger.info(f"Loading audio model from {model_path} on {DEVICE}")
        self.processor = AutoProcessor.from_pretrained(model_path)
        dtype = get_torch_dtype()
        model = BarkModel.from_pretrained(model_path, torch_dtype=dtype).to(DEVICE)
        try:
            model = model.to_bettertransformer()
        except Exception as exc:
            logger.debug(f"BetterTransformer not applied: {exc}")
        self.model = model
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        model_info = registry.get_active_model("audio")
        if not model_info:
            return EngineResponse(False, error="No active audio model selected.")
        try:
            self._load(model_info.local_path)
            token = request.cancel_token
            if token:
                token.raise_if_cancelled()

            inputs = self.processor(request.prompt, return_tensors="pt").to(DEVICE)
            with torch.no_grad():
                audio = self.model.generate(**inputs)
            audio_np: np.ndarray = audio.cpu().numpy().squeeze()
            sample_rate = self.model.generation_config.sample_rate

            out_path = os.path.join(tempfile.gettempdir(), "pyforge_audio_output.wav")
            wavfile.write(out_path, sample_rate, audio_np)

            return EngineResponse(
                True,
                out_path,
                metadata={
                    "prompt": request.prompt,
                    "model": model_info.name,
                    "sample_rate": sample_rate,
                    "duration_s": float(audio_np.shape[-1]) / sample_rate,
                },
            )
        except CancelledError:
            return EngineResponse(False, error="Cancelled.")
        except Exception as exc:
            logger.error(f"Audio generation failed: {exc}")
            return EngineResponse(False, error=str(exc))


engine_audio = AudioEngine()
