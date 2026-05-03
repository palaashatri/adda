class EngineRequest:
    """Standardized request object for all engines."""
    def __init__(self, prompt: str, model_id: str, modality: str, **kwargs):
        self.prompt = prompt
        self.model_id = model_id
        self.modality = modality
        self.kwargs = kwargs

class EngineResponse:
    """Standardized response object for all engines."""
    def __init__(self, success: bool, output: any, metadata: dict = None, error: str = None):
        self.success = success
        self.output = output
        self.metadata = metadata or {}
        self.error = error
