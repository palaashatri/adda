"""Image engine: txt2img, img2img, inpainting, upscaling via diffusers."""
from __future__ import annotations

import os
from pathlib import Path
from typing import Optional

import torch
from PIL import Image

from core.device import DEVICE, get_torch_dtype, should_compile
from core.logger import logger
from core.scheduler import CancelledError, EngineRequest, EngineResponse
from models.registry import registry


def _apply_optimizations(pipe: object) -> None:
    """Best-effort memory + speed knobs. Each is wrapped because availability varies."""
    if DEVICE != "cpu":
        for name in (
            "enable_attention_slicing",
            "enable_vae_slicing",
            "enable_vae_tiling",
        ):
            try:
                getattr(pipe, name)()
            except Exception as exc:
                logger.debug(f"{name} not applied: {exc}")
        if DEVICE == "cuda":
            try:
                pipe.enable_xformers_memory_efficient_attention()
            except Exception:
                logger.debug("xformers not available; SDPA will be used.")
            try:
                pipe.enable_model_cpu_offload()
            except Exception as exc:
                logger.debug(f"CPU offload skipped: {exc}")
    if should_compile():
        try:
            pipe.unet = torch.compile(pipe.unet, mode="reduce-overhead", fullgraph=False)
            logger.info("torch.compile applied to UNet")
        except Exception as exc:
            logger.debug(f"torch.compile failed (continuing eagerly): {exc}")


class ImageEngine:
    """Stateful diffusers wrapper covering all four image modes."""

    def __init__(self) -> None:
        self.pipeline = None
        self.current_model_path: Optional[str] = None
        self.current_mode: Optional[str] = None

    def _load(self, model_path: str, mode: str) -> None:
        cache_key = f"{model_path}::{mode}"
        if self.current_model_path == cache_key and self.pipeline is not None:
            return
        cached = registry.get_cached_pipeline(cache_key)
        if cached is not None:
            self.pipeline = cached
            self.current_model_path = cache_key
            self.current_mode = mode
            return

        from diffusers import (
            StableDiffusionImg2ImgPipeline,
            StableDiffusionInpaintPipeline,
            StableDiffusionPipeline,
            StableDiffusionUpscalePipeline,
        )

        logger.info(f"Loading image pipeline mode={mode} from {model_path} on {DEVICE}")
        dtype = get_torch_dtype()
        cls_by_mode = {
            "txt2img": StableDiffusionPipeline,
            "img2img": StableDiffusionImg2ImgPipeline,
            "inpaint": StableDiffusionInpaintPipeline,
            "upscale": StableDiffusionUpscalePipeline,
        }
        cls = cls_by_mode.get(mode, StableDiffusionPipeline)
        pipe = cls.from_pretrained(model_path, torch_dtype=dtype, safety_checker=None).to(DEVICE)
        _apply_optimizations(pipe)
        self.pipeline = pipe
        self.current_model_path = cache_key
        self.current_mode = mode
        registry.cache_pipeline(cache_key, pipe)

    def run(self, request: EngineRequest) -> EngineResponse:
        try:
            mode = request.kwargs.get("mode", "txt2img")
            model_info = registry.get_active_model("image")
            if not model_info:
                return EngineResponse(False, error="No active image model selected.")

            self._load(model_info.local_path, mode)

            token = request.cancel_token
            if token:
                token.raise_if_cancelled()

            steps = int(request.kwargs.get("steps", 20))
            guidance = float(request.kwargs.get("guidance", 7.5))
            seed = request.kwargs.get("seed")
            width = int(request.kwargs.get("width", 512))
            height = int(request.kwargs.get("height", 512))
            init_image = request.kwargs.get("init_image")
            mask_image = request.kwargs.get("mask_image")
            strength = float(request.kwargs.get("strength", 0.75))

            generator = None
            if seed not in (None, "", "random"):
                try:
                    generator = torch.Generator(device=DEVICE if DEVICE != "mps" else "cpu").manual_seed(int(seed))
                except Exception as exc:
                    logger.warning(f"Bad seed {seed!r}, ignoring: {exc}")

            def step_callback(step: int, _t: int, _latents: torch.Tensor) -> None:
                if token:
                    token.raise_if_cancelled()

            common = {
                "prompt": request.prompt,
                "num_inference_steps": steps,
                "guidance_scale": guidance,
                "generator": generator,
                "callback": step_callback,
                "callback_steps": 1,
            }

            if mode == "txt2img":
                result = self.pipeline(width=width, height=height, **common)
            elif mode == "img2img":
                if init_image is None:
                    return EngineResponse(False, error="img2img requires an init_image.")
                result = self.pipeline(image=init_image, strength=strength, **common)
            elif mode == "inpaint":
                if init_image is None or mask_image is None:
                    return EngineResponse(False, error="inpaint requires init_image and mask_image.")
                result = self.pipeline(image=init_image, mask_image=mask_image, **common)
            elif mode == "upscale":
                if init_image is None:
                    return EngineResponse(False, error="upscale requires an init_image.")
                result = self.pipeline(image=init_image, **common)
            else:
                return EngineResponse(False, error=f"Unknown image mode: {mode}")

            image: Image.Image = result.images[0]
            return EngineResponse(
                True,
                image,
                metadata={
                    "prompt": request.prompt,
                    "model": model_info.name,
                    "mode": mode,
                    "steps": steps,
                    "guidance": guidance,
                    "seed": seed,
                    "width": image.width,
                    "height": image.height,
                },
            )
        except CancelledError:
            return EngineResponse(False, error="Cancelled.")
        except Exception as exc:
            logger.error(f"Image generation failed: {exc}")
            return EngineResponse(False, error=str(exc))


engine_image = ImageEngine()
