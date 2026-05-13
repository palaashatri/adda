import torch
from core.logger import logger

def get_device() -> str:
    """Detects and returns the best available device."""
    if torch.cuda.is_available():
        logger.info("CUDA detected.")
        return "cuda"
    elif hasattr(torch.backends, 'mps') and torch.backends.mps.is_available():
        logger.info("MPS (Apple Silicon) detected.")
        return "mps"
    else:
        logger.info("No GPU detected. Falling back to CPU.")
        return "cpu"

DEVICE = get_device()
