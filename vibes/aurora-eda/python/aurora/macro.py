"""I3 — Macro recording and playback (pure-Python; mirrors core::MacroRecorder)."""

from dataclasses import dataclass, field
from typing import List


@dataclass
class MacroAction:
    verb: str
    args: List[str] = field(default_factory=list)


class MacroRecorder:
    def __init__(self):
        self._actions: List[MacroAction] = []
        self._recording: bool = False

    def start(self): self._recording = True
    def stop(self): self._recording = False
    def is_recording(self) -> bool: return self._recording

    def record(self, verb: str, *args):
        if self._recording:
            self._actions.append(MacroAction(verb, [repr(a) for a in args]))

    def to_python(self) -> str:
        lines = ["import aurora", "", "def replay(view):"]
        if not self._actions:
            lines.append("    pass")
        else:
            for a in self._actions:
                lines.append(f"    view.{a.verb}({', '.join(a.args)})")
        return "\n".join(lines)

    def save(self, path: str):
        with open(path, "w") as f:
            f.write(self.to_python())
