import os
from llama_cpp import Llama

class LLMEngine:
    def __init__(self, model_path=None):
        self.model_path = model_path
        self.model = None
        if model_path and os.path.exists(model_path):
            self.load_model(model_path)

    def load_model(self, model_path):
        print(f"Loading LLM from {model_path}...")
        self.model_path = model_path
        # n_ctx is context window, n_gpu_layers depends on hardware
        self.model = Llama(model_path=model_path, n_ctx=2048, n_gpu_layers=-1)
        print("LLM loaded successfully.")

    def generate(self, prompt, temperature=0.7, top_p=0.9, max_tokens=512, stop=None):
        if not self.model:
            return "Error: No model loaded."
        
        output = self.model(
            prompt,
            max_tokens=max_tokens,
            temperature=temperature,
            top_p=top_p,
            stop=stop,
            echo=False
        )
        return output['choices'][0]['text']

    def stream_generate(self, prompt, temperature=0.7, top_p=0.9, max_tokens=512, stop=None):
        if not self.model:
            yield "Error: No model loaded."
            return

        stream = self.model(
            prompt,
            max_tokens=max_tokens,
            temperature=temperature,
            top_p=top_p,
            stop=stop,
            echo=False,
            stream=True
        )
        for chunk in stream:
            token = chunk['choices'][0]['text']
            yield token
