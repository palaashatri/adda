from core.scheduler import EngineRequest, EngineResponse
from core.logger import logger
import time
from models.registry import registry
from transformers import pipeline
from core.device import DEVICE
import torch

class LLMEngine:
    def __init__(self):
        self.pipe = None
        self.current_model_path = None

    def load_model(self, model_path: str):
        if self.current_model_path == model_path and self.pipe is not None:
            return

        logger.info(f"Loading LLM from {model_path} onto {DEVICE}")
        dtype = torch.float16 if DEVICE != "cpu" else torch.float32
        
        self.pipe = pipeline(
            "text-generation", 
            model=model_path, 
            device=0 if DEVICE=="cuda" else -1, # Simplification
            torch_dtype=dtype
        )
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        try:
            model_info = registry.get_active_model("llm")
            if not model_info:
                return EngineResponse(False, None, error="No active LLM selected.")

            self.load_model(model_info.local_path)
            
            prompt = request.prompt
            max_new_tokens = request.kwargs.get("max_new_tokens", 50)
            
            logger.info(f"Generating text for prompt: '{prompt}'")
            
            result = self.pipe(prompt, max_new_tokens=max_new_tokens, do_sample=True)
            generated_text = result[0]['generated_text']
            
            return EngineResponse(True, generated_text, metadata={"prompt": prompt, "model": model_info.name})

        except Exception as e:
            logger.error(f"LLM generation failed: {e}")
            return EngineResponse(False, None, error=str(e))

engine_llm = LLMEngine()
