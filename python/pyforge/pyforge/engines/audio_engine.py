import torch
import os

try:
    from audiocraft.models import MusicGen
    from audiocraft.data.audio import audio_write
    AUDIOCRAFT_AVAILABLE = True
except ImportError:
    AUDIOCRAFT_AVAILABLE = False

class AudioEngine:
    def __init__(self, model_id='facebook/musicgen-small'):
        self.model_id = model_id
        self.model = None
        # Determine device: CUDA > MPS > CPU
        if torch.cuda.is_available():
            self.device = "cuda"
        elif torch.backends.mps.is_available():
            self.device = "mps"
        else:
            self.device = "cpu"

    def load_model(self, model_id=None):
        if not AUDIOCRAFT_AVAILABLE:
            print("AudioEngine: audiocraft not installed.")
            return

        if model_id:
            self.model_id = model_id
        
        print(f"Loading Audio Model {self.model_id} on {self.device}...")
        try:
            self.model = MusicGen.get_pretrained(self.model_id, device=self.device)
            print("Audio Model loaded successfully.")
        except Exception as e:
            print(f"Error loading audio model: {e}")
            self.model = None

    def generate(self, prompt, duration=10):
        if not AUDIOCRAFT_AVAILABLE:
            return "Error: audiocraft library not installed. Music generation unavailable."

        if not self.model:
            print("Error: No audio model loaded. Loading default...")
            self.load_model()
            if not self.model:
                return None
        
        self.model.set_generation_params(duration=duration)
        
        try:
            wav = self.model.generate([prompt]) # wav: [B, C, T]
            return wav[0] # Return the first (and only) sample
        except Exception as e:
            print(f"Error generating audio: {e}")
            return None

    def save_audio(self, wav, output_path):
        if not AUDIOCRAFT_AVAILABLE or wav is None:
            return False
        
        try:
            audio_write(output_path, wav.cpu(), self.model.sample_rate, strategy="loudness", loudness_compressor=True)
            return True
        except Exception as e:
            print(f"Error saving audio: {e}")
            return False
