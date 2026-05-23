"""LLM engine: llama.cpp (GGUF) if a .gguf is in the model dir, else transformers."""
from __future__ import annotations

from pathlib import Path
from typing import Optional

import torch

from core.device import DEVICE, get_torch_dtype
from core.logger import logger
from core.scheduler import CancelledError, EngineRequest, EngineResponse
from models.registry import registry


def _find_gguf(model_dir: str) -> Optional[Path]:
    """Return the first .gguf file inside `model_dir`, if any."""
    p = Path(model_dir)
    if not p.exists():
        return None
    for f in p.rglob("*.gguf"):
        return f
    return None


class LLMEngine:
    """Dual-backend LLM wrapper: prefers llama.cpp (GGUF) for speed/RAM."""

    def __init__(self) -> None:
        self.backend: Optional[str] = None
        self.handle = None
        self.current_model_path: Optional[str] = None

    def _load(self, model_path: str) -> None:
        if self.current_model_path == model_path and self.handle is not None:
            return

        gguf_path = _find_gguf(model_path)
        if gguf_path is not None:
            try:
                from llama_cpp import Llama
                logger.info(f"Loading LLM via llama.cpp: {gguf_path}")
                n_gpu_layers = -1 if DEVICE == "cuda" else 0
                self.handle = Llama(
                    model_path=str(gguf_path),
                    n_ctx=4096,
                    n_gpu_layers=n_gpu_layers,
                    verbose=False,
                )
                self.backend = "llama_cpp"
                self.current_model_path = model_path
                return
            except Exception as exc:
                logger.warning(f"llama-cpp-python unavailable ({exc}); using transformers.")

        from transformers import pipeline
        logger.info(f"Loading LLM via transformers from {model_path}")
        kwargs = {
            "model": model_path,
            "device": 0 if DEVICE == "cuda" else -1,
            "torch_dtype": get_torch_dtype(),
        }
        if DEVICE == "cuda":
            try:
                self.handle = pipeline(
                    "text-generation",
                    attn_implementation="flash_attention_2",
                    **kwargs,
                )
            except Exception:
                self.handle = pipeline("text-generation", **kwargs)
        else:
            self.handle = pipeline("text-generation", **kwargs)
        self.backend = "transformers"
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        model_info = registry.get_active_model("llm")
        if not model_info:
            return EngineResponse(False, error="No active LLM selected.")
        try:
            self._load(model_info.local_path)
            token = request.cancel_token
            if token:
                token.raise_if_cancelled()

            max_new_tokens = int(request.kwargs.get("max_new_tokens", 128))
            temperature = float(request.kwargs.get("temperature", 0.8))
            top_p = float(request.kwargs.get("top_p", 0.95))

            if self.backend == "llama_cpp":
                output = self.handle(
                    request.prompt,
                    max_tokens=max_new_tokens,
                    temperature=temperature,
                    top_p=top_p,
                )
                text = output["choices"][0]["text"]
                generated = request.prompt + text
            else:
                result = self.handle(
                    request.prompt,
                    max_new_tokens=max_new_tokens,
                    temperature=temperature,
                    top_p=top_p,
                    do_sample=True,
                )
                generated = result[0]["generated_text"]

            return EngineResponse(
                True,
                generated,
                metadata={
                    "prompt": request.prompt,
                    "model": model_info.name,
                    "backend": self.backend,
                    "max_new_tokens": max_new_tokens,
                },
            )
        except CancelledError:
            return EngineResponse(False, error="Cancelled.")
        except Exception as exc:
            logger.error(f"LLM generation failed: {exc}")
            return EngineResponse(False, error=str(exc))


engine_llm = LLMEngine()
