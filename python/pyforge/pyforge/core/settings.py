import os
import json
from pathlib import Path

class Settings:
    def __init__(self):
        self.app_dir = Path.home() / ".pyforge"
        self.app_dir.mkdir(exist_ok=True)
        self.settings_file = self.app_dir / "settings.json"
        
        self.defaults = {
            "theme": "darkly",
            "model_cache": str(self.app_dir / "models"),
            "gpu_enabled": True,
            "api_server_enabled": False,
            "api_port": 8000,
            "max_vram_usage": 0.8
        }
        
        self.current_settings = self.load_settings()

    def load_settings(self):
        if self.settings_file.exists():
            try:
                with open(self.settings_file, "r") as f:
                    return {**self.defaults, **json.load(f)}
            except Exception as e:
                print(f"Error loading settings: {e}")
        return self.defaults.copy()

    def save_settings(self):
        with open(self.settings_file, "w") as f:
            json.dump(self.current_settings, f, indent=4)

    def get(self, key):
        return self.current_settings.get(key, self.defaults.get(key))

    def set(self, key, value):
        self.current_settings[key] = value
        self.save_settings()

settings = Settings()
