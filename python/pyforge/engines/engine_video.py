"""Video engine: AnimateDiff via diffusers if installed, else honest error."""
from __future__ import annotations

import os
import tempfile
from typing import Optional

import torch

from core.device import DEVICE, get_torch_dtype
from core.logger import logger
from core.scheduler import CancelledError, EngineRequest, EngineResponse
from models.registry import registry


class VideoEngine:
    """Best-effort AnimateDiff wrapper. Returns a clean error when no model is set."""

    def __init__(self) -> None:
        self.pipeline = None
        self.current_model_path: Optional[str] = None

    def _load(self, model_path: str) -> None:
        if self.current_model_path == model_path and self.pipeline is not None:
            return
        cached = registry.get_cached_pipeline(model_path)
        if cached is not None:
            self.pipeline = cached
            self.current_model_path = model_path
            return

        from diffusers import AnimateDiffPipeline, MotionAdapter

        logger.info(f"Loading video pipeline from {model_path} on {DEVICE}")
        dtype = get_torch_dtype()
        try:
            adapter = MotionAdapter.from_pretrained(model_path, torch_dtype=dtype)
            pipe = AnimateDiffPipeline.from_pretrained(
                model_path, motion_adapter=adapter, torch_dtype=dtype
            ).to(DEVICE)
        except Exception:
            # Treat the directory itself as a full AnimateDiff pipeline checkpoint.
            pipe = AnimateDiffPipeline.from_pretrained(model_path, torch_dtype=dtype).to(DEVICE)

        for name in ("enable_vae_slicing", "enable_vae_tiling"):
            try:
                getattr(pipe, name)()
            except Exception:
                pass
        if DEVICE == "cuda":
            try:
                pipe.enable_model_cpu_offload()
            except Exception as exc:
                logger.debug(f"video cpu offload skipped: {exc}")

        self.pipeline = pipe
        self.current_model_path = model_path
        registry.cache_pipeline(model_path, pipe)

    def run(self, request: EngineRequest) -> EngineResponse:
        model_info = registry.get_active_model("video")
        if not model_info:
            return EngineResponse(
                False,
                error="No active video model selected. Install one via the Downloader tab.",
            )
        try:
            self._load(model_info.local_path)
            token = request.cancel_token
            if token:
                token.raise_if_cancelled()

            num_frames = int(request.kwargs.get("num_frames", 16))
            fps = int(request.kwargs.get("fps", 8))
            steps = int(request.kwargs.get("steps", 25))
            guidance = float(request.kwargs.get("guidance", 7.5))

            def step_callback(step: int, _t: int, _latents: torch.Tensor) -> None:
                if token:
                    token.raise_if_cancelled()

            result = self.pipeline(
                prompt=request.prompt,
                num_frames=num_frames,
                num_inference_steps=steps,
                guidance_scale=guidance,
                callback=step_callback,
                callback_steps=1,
            )
            frames = result.frames[0]

            try:
                from diffusers.utils import export_to_gif
                out_path = os.path.join(tempfile.gettempdir(), "pyforge_video.gif")
                export_to_gif(frames, out_path, fps=fps)
            except Exception as exc:
                logger.warning(f"GIF export failed, returning raw frames: {exc}")
                out_path = ""

            return EngineResponse(
                True,
                out_path or frames,
                metadata={
                    "prompt": request.prompt,
                    "model": model_info.name,
                    "num_frames": num_frames,
                    "fps": fps,
                },
            )
        except CancelledError:
            return EngineResponse(False, error="Cancelled.")
        except Exception as exc:
            logger.error(f"Video generation failed: {exc}")
            return EngineResponse(False, error=str(exc))


engine_video = VideoEngine()
