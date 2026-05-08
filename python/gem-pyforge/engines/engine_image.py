import torch
from pathlib import Path
from PIL import Image
from diffusers import StableDiffusionPipeline
from core.scheduler import EngineRequest, EngineResponse
from core.device import DEVICE
from core.logger import logger
from models.registry import registry

class ImageEngine:
    def __init__(self):
        self.pipeline = None
        self.current_model_path = None

    def load_model(self, model_path: str):
        if self.current_model_path == model_path and self.pipeline is not None:
            return

        logger.info(f"Loading Image model from {model_path} onto {DEVICE}")
        dtype = torch.float16 if DEVICE != "cpu" else torch.float32
        
        self.pipeline = StableDiffusionPipeline.from_pretrained(
            model_path, 
            torch_dtype=dtype,
            safety_checker=None
        ).to(DEVICE)
        
        # Optional: memory optimizations
        if DEVICE != "cpu":
            self.pipeline.enable_attention_slicing()

        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        try:
            model_info = registry.get_active_model("image")
            if not model_info:
                return EngineResponse(False, None, error="No active image model selected.")

            self.load_model(model_info.local_path)
            
            prompt = request.prompt
            num_inference_steps = request.kwargs.get("steps", 20)
            guidance_scale = request.kwargs.get("guidance", 7.5)
            
            logger.info(f"Generating image for prompt: '{prompt}'")
            
            # This is a blocking call; it should be run in the task queue
            image = self.pipeline(
                prompt=prompt,
                num_inference_steps=num_inference_steps,
                guidance_scale=guidance_scale
            ).images[0]
            
            return EngineResponse(True, image, metadata={"prompt": prompt, "model": model_info.name})

        except Exception as e:
            logger.error(f"Image generation failed: {e}")
            return EngineResponse(False, None, error=str(e))

engine_image = ImageEngine()
