from core.scheduler import EngineRequest, EngineResponse
from core.logger import logger
import time
from models.registry import registry
from core.device import DEVICE
from transformers import AutoProcessor, BarkModel
import torch
import scipy.io.wavfile as wavfile
import numpy as np
import tempfile
import os

class AudioEngine:
    def __init__(self):
        self.processor = None
        self.model = None
        self.current_model_path = None

    def load_model(self, model_path: str):
        if self.current_model_path == model_path and self.model is not None:
            return

        logger.info(f"Loading Audio model from {model_path} onto {DEVICE}")
        
        # Check if the folder is valid Bark model (has config.json)
        self.processor = AutoProcessor.from_pretrained(model_path)
        self.model = BarkModel.from_pretrained(model_path).to(DEVICE)
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        try:
            model_info = registry.get_active_model("audio")
            if not model_info:
                return EngineResponse(False, None, error="No active audio model selected.")

            self.load_model(model_info.local_path)
            
            prompt = request.prompt
            logger.info(f"Generating audio for prompt: '{prompt}'")
            
            inputs = self.processor(prompt, return_tensors="pt").to(DEVICE)
            
            with torch.no_grad():
                audio_array = self.model.generate(**inputs)
            
            audio_array = audio_array.cpu().numpy().squeeze()
            
            # Save to a temporary file
            temp_dir = tempfile.gettempdir()
            output_path = os.path.join(temp_dir, "pyforge_audio_output.wav")
            sample_rate = self.model.generation_config.sample_rate
            
            wavfile.write(output_path, sample_rate, audio_array)
            
            return EngineResponse(True, output_path, metadata={"prompt": prompt, "model": model_info.name})

        except Exception as e:
            logger.error(f"Audio generation failed: {e}")
            return EngineResponse(False, None, error=str(e))

engine_audio = AudioEngine()
