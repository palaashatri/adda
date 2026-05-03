from core.scheduler import EngineRequest, EngineResponse
from core.logger import logger
from models.registry import registry
from transformers import pipeline
from core.device import DEVICE
import torch

class SpeechEngine:
    def __init__(self):
        self.pipe = None
        self.current_model_path = None

    def load_model(self, model_path: str):
        if self.current_model_path == model_path and self.pipe is not None:
            return

        logger.info(f"Loading Speech model from {model_path} onto {DEVICE}")
        dtype = torch.float16 if DEVICE != "cpu" else torch.float32
        
        self.pipe = pipeline(
            "automatic-speech-recognition", 
            model=model_path,
            device=0 if DEVICE=="cuda" else -1,
            torch_dtype=dtype
        )
        self.current_model_path = model_path

    def run(self, request: EngineRequest) -> EngineResponse:
        try:
            model_info = registry.get_active_model("speech")
            if not model_info:
                return EngineResponse(False, None, error="No active speech model selected.")

            self.load_model(model_info.local_path)
            
            audio_path = request.prompt # Here prompt acts as the path to the audio file
            logger.info(f"Transcribing audio from: '{audio_path}'")
            
            result = self.pipe(audio_path)
            transcription = result["text"]
            
            return EngineResponse(True, transcription, metadata={"audio": audio_path, "model": model_info.name})

        except Exception as e:
            logger.error(f"Speech transcription failed: {e}")
            return EngineResponse(False, None, error=str(e))

engine_speech = SpeechEngine()
