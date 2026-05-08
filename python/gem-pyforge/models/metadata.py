import json
from dataclasses import dataclass, asdict
from typing import List, Optional
from pathlib import Path

@dataclass
class ModelInfo:
    name: str
    type: str
    hf_id: str
    version: str
    size: float
    tags: List[str]
    local_path: str
    download_date: str

    def save(self, path: Path):
        """Saves metadata to a JSON file."""
        with open(path, "w") as f:
            json.dump(asdict(self), f, indent=2)

    @classmethod
    def load(cls, path: Path) -> "ModelInfo":
        """Loads metadata from a JSON file."""
        with open(path, "r") as f:
            data = json.load(f)
        return cls(**data)
