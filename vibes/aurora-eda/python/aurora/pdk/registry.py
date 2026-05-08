"""In-process registry for Python PCells."""

from __future__ import annotations

from typing import Type

from .pcell_base import PcellBase

_PCELLS: dict[str, Type[PcellBase]] = {}


def register_pcell(name: str, cls: Type[PcellBase]) -> None:
    if not issubclass(cls, PcellBase):
        raise TypeError(f"{cls.__name__} must inherit from PcellBase")
    _PCELLS[name] = cls


def get_pcell(name: str) -> Type[PcellBase]:
    return _PCELLS[name]


def registered_pcells() -> dict[str, Type[PcellBase]]:
    return dict(_PCELLS)
