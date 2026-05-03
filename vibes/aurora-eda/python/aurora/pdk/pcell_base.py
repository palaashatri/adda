"""Base interface for Python-defined parameterized cells."""


class PcellBase:
    """Base class for Python PCells."""

    @classmethod
    def parameters(cls) -> dict:
        """Return parameter definitions keyed by parameter name."""
        return {}

    @classmethod
    def generate_layout(cls, cell, tech, params):
        raise NotImplementedError(f"{cls.__name__}.generate_layout() is required")

    @classmethod
    def generate_schematic(cls, cell, tech, params):
        return None
