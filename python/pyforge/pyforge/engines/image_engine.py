import torch
from diffusers import StableDiffusionPipeline
import os

class ImageEngine:
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
        print(f"Loading Image Model from {model_id} on {self.device}...")
        self.model_id = model_id
        
        try:
            # Check if it's a local directory or a HF model ID
            if os.path.exists(model_id):
                self.pipe = StableDiffusionPipeline.from_pretrained(
                    model_id, 
                    torch_dtype=self.torch_dtype,
                    use_safetensors=True if any(f.endswith('.safetensors') for f in os.listdir(model_id) if os.path.isfile(os.path.join(model_id, f))) else False
                ).to(self.device)
            else:
                self.pipe = StableDiffusionPipeline.from_pretrained(
                    model_id, 
                    torch_dtype=self.torch_dtype,
                    use_safetensors=True
                ).to(self.device)
            
            # Enable memory optimizations if on CUDA
            if self.device == "cuda":
                self.pipe.enable_attention_slicing()
                
            print("Image Model loaded successfully.")
        except Exception as e:
            print(f"Error loading model: {e}")
            self.pipe = None

    def generate(self, prompt, negative_prompt=None, num_inference_steps=50, guidance_scale=7.5, width=512, height=512, seed=None):
        if not self.pipe:
            print("Error: No image model loaded.")
            return None
        
        generator = None
        if seed is not None and seed != -1:
            generator = torch.Generator(device=self.device).manual_seed(seed)
        
        # Ensure dimensions are multiples of 8
        width = (width // 8) * 8
        height = (height // 8) * 8
        
        image = self.pipe(
            prompt,
            negative_prompt=negative_prompt,
            num_inference_steps=int(num_inference_steps),
            guidance_scale=float(guidance_scale),
            width=int(width),
            height=int(height),
            generator=generator
        ).images[0]
        
        return image
