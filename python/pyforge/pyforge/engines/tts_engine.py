import torch
import os

try:
    from TTS.api import TTS
    TTS_AVAILABLE = True
except ImportError:
    TTS_AVAILABLE = False

class TTSEngine:
    def __init__(self, model_id="tts_models/multilingual/multi-dataset/xtts_v2"):
        self.model_id = model_id
        self.tts = None
        # Determine device: CUDA > MPS > CPU
        if torch.cuda.is_available():
            self.device = "cuda"
        elif torch.backends.mps.is_available():
            self.device = "mps"
        else:
            self.device = "cpu"

    def load_model(self, model_id=None):
        if not TTS_AVAILABLE:
            print("TTSEngine: TTS not installed.")
            return

        if model_id:
            self.model_id = model_id
        
        print(f"Loading TTS Model {self.model_id} on {self.device}...")
        try:
            self.tts = TTS(model_name=self.model_id).to(self.device)
            print("TTS Model loaded successfully.")
        except Exception as e:
            print(f"Error loading TTS model: {e}")
            self.tts = None

    def generate(self, text, speaker=None, language="en", output_path="output.wav", speed=1.0):
        if not TTS_AVAILABLE:
            return False

        if not self.tts:
            print("Error: No TTS model loaded. Loading default...")
            self.load_model()
            if not self.tts:
                return False
        
        try:
            # XTTS requires a speaker_wav or speaker name
            if "xtts" in self.model_id.lower():
                speakers = self.tts.speakers if hasattr(self.tts, 'speakers') else None
                if speakers and not speaker:
                    speaker = speakers[0]
                
                self.tts.tts_to_file(
                    text=text,
                    speaker=speaker,
                    language=language,
                    file_path=output_path,
                    speed=float(speed)
                )
            else:
                self.tts.tts_to_file(
                    text=text,
                    file_path=output_path
                )
            return True
        except Exception as e:
            print(f"Error generating speech: {e}")
            return False

    def get_speakers(self):
        if TTS_AVAILABLE and self.tts and hasattr(self.tts, 'speakers'):
            return self.tts.speakers
        return []
