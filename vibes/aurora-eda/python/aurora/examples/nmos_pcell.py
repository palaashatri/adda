"""Example NMOS PCell for aurora-eda.

Demonstrates how to write a parameterized cell using PcellBase.
The generate_layout() method creates a single-finger NMOS transistor
using three layers: diffusion, poly-gate, and metal1 contacts.

Registration example::

    from aurora.pdk.registry import register_pcell
    from aurora.examples.nmos_pcell import NmosPcell
    register_pcell("NMOS", NmosPcell)

Programmatic invocation example (requires compiled aurora extension)::

    from aurora import _aurora  # compiled C++ module
    import aurora.examples.nmos_pcell as nmos_mod

    lib = _aurora.CellLib("mylib")
    tech = _aurora.TechDatabase()
    cell = lib.create_cell("my_nmos")
    view = cell.create_view(_aurora.ViewType.Layout)

    params = {"w": "1000", "l": "200", "fingers": "1"}
    NmosPcell.generate_layout(view, tech, params)
"""

from __future__ import annotations

from aurora.pdk.pcell_base import PcellBase


class NmosPcell(PcellBase):
    """Single-finger NMOS transistor PCell.

    Parameter reference (all dimensions in nm unless noted):
        w        — channel width  (default 1000 nm = 1 µm)
        l        — channel length (default 100 nm)
        fingers  — number of parallel gate fingers (default 1)
    """

    @classmethod
    def parameters(cls) -> dict:
        return {
            "w":       {"default": "1000", "description": "Channel width (nm)"},
            "l":       {"default": "100",  "description": "Channel length (nm)"},
            "fingers": {"default": "1",    "description": "Gate finger count"},
        }

    @classmethod
    def generate_layout(cls, view, tech, params: dict) -> None:
        """Populate *view* with NMOS geometry.

        The view object is either a C++ aurora DbView (from the compiled
        extension) or a duck-typed stand-in used during testing.
        """
        w       = int(params.get("w",       "1000"))
        l       = int(params.get("l",       "100"))
        fingers = int(params.get("fingers", "1"))

        # --- resolve layer IDs via the tech database (optional) ---
        # If tech is not loaded, we fall back to layer name strings and
        # expect the caller to supply a library with matching layers.
        def layer_id(name: str) -> object:
            if tech is not None:
                info = tech.find_layer_by_name(name)
                if info is not None:
                    return info.id
            return name  # fallback: caller maps name→id

        diff_id  = layer_id("diff")
        poly_id  = layer_id("poly")
        met1_id  = layer_id("metal1")

        # --- geometry constants (nm) ---
        sd_ext   = 200          # source/drain extension beyond channel
        sd_width = w            # S/D diffusion width equals channel width
        gate_ext = 100          # poly extends past diffusion
        contact_size = 100
        contact_enc  = 50

        pitch = l + 2 * sd_ext  # one finger pitch

        for f in range(fingers):
            ox = f * pitch  # finger origin x

            # Diffusion rectangle: full S/D region for this finger
            diff_left   = ox
            diff_right  = ox + pitch
            diff_bottom = 0
            diff_top    = sd_width
            _rect(view, diff_id,
                  diff_left, diff_bottom, diff_right, diff_top)

            # Poly gate
            gate_left   = ox + sd_ext
            gate_right  = ox + sd_ext + l
            _rect(view, poly_id,
                  gate_left,  diff_bottom - gate_ext,
                  gate_right, diff_top    + gate_ext)

            # Source contact (left side)
            cx_s = ox + sd_ext // 2 - contact_size // 2
            _rect(view, met1_id,
                  cx_s - contact_enc, contact_enc,
                  cx_s + contact_size + contact_enc,
                  sd_width - contact_enc)

            # Drain contact (right side)
            cx_d = ox + sd_ext + l + sd_ext // 2 - contact_size // 2
            _rect(view, met1_id,
                  cx_d - contact_enc, contact_enc,
                  cx_d + contact_size + contact_enc,
                  sd_width - contact_enc)

    @classmethod
    def generate_schematic(cls, view, tech, params: dict) -> None:
        return None


# ──────────────────────────────────────────────────────────────────────────────
# Helper — works whether view is the real C++ object or a dict-based mock.
# ──────────────────────────────────────────────────────────────────────────────

def _rect(view, layer_id, left, bottom, right, top) -> None:
    """Create a rectangle in *view*, tolerating both C++ and duck-typed views."""
    try:
        from aurora._aurora import Box  # type: ignore[import]
        view.create_rect(layer_id, Box(left, bottom, right, top))
    except Exception:
        # Duck-typed view used in unit tests
        shapes = getattr(view, "_shapes", None)
        if shapes is None:
            view._shapes = []  # type: ignore[attr-defined]
            shapes = view._shapes
        shapes.append({"layer": layer_id, "box": (left, bottom, right, top)})
