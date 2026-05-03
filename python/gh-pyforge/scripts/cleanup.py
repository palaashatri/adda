from __future__ import annotations

import shutil
from pathlib import Path


def cleanup_temp() -> None:
    """Remove temporary PyForge build artifacts."""

    for path in [Path("build"), Path("dist"), Path("__pycache__")]:
        if path.exists():
            if path.is_dir():
                shutil.rmtree(path)
            else:
                path.unlink()
    print("Cleanup complete.")


if __name__ == "__main__":
    cleanup_temp()
