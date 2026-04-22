import os
import json
from pathlib import Path
from pyforge.core.settings import settings

class ModelRegistry:
    def __init__(self):
        self.model_dir = Path(settings.get("model_cache"))
        self.model_dir.mkdir(parents=True, exist_ok=True)
        self.registry_file = self.model_dir / "registry.json"
        self.models = self.load_registry()

    def load_registry(self):
        if self.registry_file.exists():
            try:
                with open(self.registry_file, "r") as f:
                    return json.load(f)
            except Exception as e:
                print(f"Error loading registry: {e}")
        return {}

    def save_registry(self):
        try:
            with open(self.registry_file, "w") as f:
                json.dump(self.models, f, indent=4)
        except Exception as e:
            print(f"Error saving registry: {e}")

    def add_model(self, model_id, metadata):
        local_path = os.path.join(str(self.model_dir), model_id.replace("/", "--"))
        metadata["local_path"] = local_path
        self.models[model_id] = metadata
        self.save_registry()

    def remove_model(self, model_id):
        if model_id in self.models:
            import shutil
            local_path = self.models[model_id].get("local_path")
            if local_path and os.path.exists(local_path):
                shutil.rmtree(local_path)
            del self.models[model_id]
            self.save_registry()

    def get_local_models(self):
        # Scan filesystem and sync with registry
        # For each folder in model_dir, check if it's a model
        # For simplicity, we'll rely on the registry for now.
        return self.models

    def is_model_downloaded(self, model_id):
        return model_id in self.models and os.path.exists(self.models[model_id].get("local_path", ""))

model_registry = ModelRegistry()
