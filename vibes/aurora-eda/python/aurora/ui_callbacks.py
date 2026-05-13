"""I4 — User-defined menu items / toolbar callbacks.

Define Python callbacks here and the GUI's "Tools > Scripted Actions" menu
auto-discovers them at startup. Each callback should accept (app) and return None.
"""

from typing import Callable, Dict


_callbacks: Dict[str, Callable] = {}


def register(label: str, hotkey: str = ""):
    """Decorator: `@register("My Tool", hotkey="Ctrl+Shift+M")`."""
    def deco(fn):
        _callbacks[label] = fn
        if hotkey:
            fn.__aurora_hotkey__ = hotkey
        return fn
    return deco


def items() -> Dict[str, Callable]:
    return dict(_callbacks)
