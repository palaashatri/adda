"""I8 — Layout automation helpers (pure-Python; works with aurora bindings when available)."""

from dataclasses import dataclass
from typing import List, Tuple


@dataclass
class RectSpec:
    layer: str
    x1: int
    y1: int
    x2: int
    y2: int


def array_place(origin: Tuple[int, int], cols: int, rows: int,
                pitch_x: int, pitch_y: int) -> List[Tuple[int, int]]:
    """Return placement coordinates for a cols x rows array starting at origin."""
    ox, oy = origin
    return [(ox + c * pitch_x, oy + r * pitch_y)
            for r in range(rows) for c in range(cols)]


def route_l_shape(p1: Tuple[int, int], p2: Tuple[int, int], width: int,
                  prefer: str = "h") -> List[RectSpec]:
    """Return rect specs that route an L-shape from p1 to p2."""
    x1, y1 = p1
    x2, y2 = p2
    half = width // 2
    if prefer == "h":
        return [
            RectSpec("metal1", min(x1, x2), y1 - half, max(x1, x2), y1 + half),
            RectSpec("metal1", x2 - half, min(y1, y2), x2 + half, max(y1, y2)),
        ]
    return [
        RectSpec("metal1", x1 - half, min(y1, y2), x1 + half, max(y1, y2)),
        RectSpec("metal1", min(x1, x2), y2 - half, max(x1, x2), y2 + half),
    ]


def bounding_box(rects: List[RectSpec]) -> Tuple[int, int, int, int]:
    if not rects:
        return (0, 0, 0, 0)
    return (min(r.x1 for r in rects), min(r.y1 for r in rects),
            max(r.x2 for r in rects), max(r.y2 for r in rects))
