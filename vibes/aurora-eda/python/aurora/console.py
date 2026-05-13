"""I1 — Python interactive shell for aurora-eda.

When the host is running, this drops into an interactive `code.InteractiveConsole`
with `aurora` already imported. From the GUI we forward stdin/stdout to a Qt
QPlainTextEdit; this module is the pure-Python REPL the UI embeds.
"""

import code
import sys
from typing import Optional


def start(locals_dict: Optional[dict] = None, banner: str = "aurora-eda Python console"):
    ns = {"__name__": "__aurora_console__"}
    try:
        import aurora  # noqa: F401
        ns["aurora"] = aurora
    except Exception:
        pass
    if locals_dict:
        ns.update(locals_dict)
    code.interact(banner=banner, local=ns)


if __name__ == "__main__":
    start()
