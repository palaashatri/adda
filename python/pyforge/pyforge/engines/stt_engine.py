import torch
import os

try:
    import whisper
    WHISPER_AVAILABLE = True
except ImportError:
    WHISPER_AVAILABLE = False

class STTEngine:
    def __init__(self, model_size="base"):
        self.model_size = model_size
        self.model = None
        # Determine device: CUDA > MPS > CPU
        if torch.cuda.is_available():
            self.device = "cuda"
        elif torch.backends.mps.is_available():
            self.device = "mps"
        else:
            self.device = "cpu"

    def load_model(self, model_size=None):
        if not WHISPER_AVAILABLE:
            print("STTEngine: whisper not installed.")
            return

        if model_size:
            self.model_size = model_size
        
        print(f"Loading Whisper Model {self.model_size} on {self.device}...")
        try:
            self.model = whisper.load_model(self.model_size, device=self.device)
            print("Whisper Model loaded successfully.")
        except Exception as e:
            print(f"Error loading Whisper model: {e}")
            self.model = None

    def transcribe(self, audio_path, language=None):
        if not WHISPER_AVAILABLE:
            return "Error: whisper library not installed."

        if not self.model:
            print("Error: No Whisper model loaded. Loading default...")
            self.load_model()
            if not self.model:
                return None
        
        try:
            result = self.model.transcribe(audio_path, language=language)
            return result["text"]
        except Exception as e:
            print(f"Error during transcription: {e}")
            return None
