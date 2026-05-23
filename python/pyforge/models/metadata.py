"""ModelInfo dataclass + JSON I/O."""
from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import List


@dataclass
class ModelInfo:
    """Metadata for an installed model."""
    name: str
    type: str
    hf_id: str
    version: str
    size: float
    tags: List[str]
    local_path: str
    download_date: str
    sha256: str = ""
    is_default: bool = False
    extra: dict = field(default_factory=dict)

    def save(self, path: Path) -> None:
        """Save metadata to a JSON file."""
        with open(path, "w", encoding="utf-8") as f:
            json.dump(asdict(self), f, indent=2)

    @classmethod
    def load(cls, path: Path) -> "ModelInfo":
        """Load metadata from a JSON file."""
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        data.setdefault("sha256", "")
        data.setdefault("is_default", False)
        data.setdefault("extra", {})
        return cls(**data)
