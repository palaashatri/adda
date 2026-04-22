import torch
from diffusers import TextToVideoSDPipeline
import os
import imageio
import numpy as np

class VideoEngine:
    def __init__(self, model_id=None):
        self.model_id = model_id
        self.pipe = None
        # Determine device: CUDA > MPS > CPU
        if torch.cuda.is_available():
            self.device = "cuda"
        elif torch.backends.mps.is_available():
            self.device = "mps"
        else:
            self.device = "cpu"
            
        self.torch_dtype = torch.float16 if self.device != "cpu" else torch.float32

        if model_id:
            self.load_model(model_id)

    def load_model(self, model_id):
        print(f"Loading Video Model from {model_id} on {self.device}...")
        self.model_id = model_id
        
        try:
            # For this implementation, we use TextToVideoSDPipeline as a default
            # but this can be extended for LTX-Video, CogVideoX, etc.
            self.pipe = TextToVideoSDPipeline.from_pretrained(
                model_id, 
                torch_dtype=self.torch_dtype,
                variant="fp16" if self.device != "cpu" else None
            ).to(self.device)
            
            # Enable memory optimizations if on CUDA
            if self.device == "cuda":
                self.pipe.enable_model_cpu_offload()
                
            print("Video Model loaded successfully.")
        except Exception as e:
            print(f"Error loading video model: {e}")
            self.pipe = None

    def generate(self, prompt, num_frames=16, num_inference_steps=50, guidance_scale=7.5, width=512, height=512, seed=None):
        if not self.pipe:
            print("Error: No video model loaded.")
            return None
        
        generator = None
        if seed is not None and seed != -1:
            generator = torch.Generator(device=self.device).manual_seed(seed)
        
        # Ensure dimensions are multiples of 8
        width = (width // 8) * 8
        height = (height // 8) * 8
        
        video_frames = self.pipe(
            prompt,
            num_frames=int(num_frames),
            num_inference_steps=int(num_inference_steps),
            guidance_scale=float(guidance_scale),
            width=int(width),
            height=int(height),
            generator=generator
        ).frames
        
        # Convert frames to uint8 for saving
        # Diffusers returns a list of PIL images or a numpy array depending on version/pipeline
        if isinstance(video_frames[0], np.ndarray):
             frames = [ (frame * 255).astype(np.uint8) for frame in video_frames]
        else:
             frames = [ np.array(frame) for frame in video_frames]
             
        return frames

    def save_video(self, frames, output_path, fps=8):
        if not frames:
            return False
        
        try:
            imageio.mimsave(output_path, frames, fps=fps)
            return True
        except Exception as e:
            print(f"Error saving video: {e}")
            return False
