from __future__ import annotations

import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any


@dataclass
class ModelInfo:
    """Metadata for a model in the local registry."""

    name: str
    modality: str
    size: int
    local_path: Path
    hf_id: str
    version: str
    tags: list[str]
    download_date: str

    def to_dict(self) -> dict[str, Any]:
        data = asdict(self)
        data["local_path"] = str(self.local_path)
        return data

    @classmethod
    def from_dict(cls, raw: dict[str, Any]) -> ModelInfo:
        return cls(
            name=str(raw["name"]),
            modality=str(raw["modality"]),
            size=int(raw["size"]),
            local_path=Path(raw["local_path"]),
            hf_id=str(raw["hf_id"]),
            version=str(raw["version"]),
            tags=[str(tag) for tag in raw.get("tags", [])],
            download_date=str(raw["download_date"]),
        )

    def write_metadata_file(self) -> None:
        metadata_file = self.local_path / "model.json"
        self.local_path.mkdir(parents=True, exist_ok=True)
        metadata_file.write_text(json.dumps(self.to_dict(), indent=2), encoding="utf-8")
